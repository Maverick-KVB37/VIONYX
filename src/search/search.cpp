#include "search.h"
#include <iostream>

namespace Search {
/*
Searcher::Searcher(Position& pos, TranspositionTable& tt)
    : pos(pos), tt(tt), stopFlag(false), nodes(0), selDepth(0) {

        //std::cerr << "DEBUG: Searcher constructor called\n";
    
    // Initialize stack
    for (int i = 0; i < MAX_PLY + 10; i++) {
        stack[i] = SearchStack();
    }
    
    //std::cerr << "DEBUG: Searcher constructor done\n";
}


Move Searcher::think(const SearchLimits& limits) {
    //std::cerr<<"DEBUG: think() called\n";
    this->limits = limits;
    this->stopFlag = false;
    this->nodes = 0;
    this->info.clear();
    this->startTime = std::chrono::steady_clock::now();
    this->selDepth = 0;

    tt.newSearch();
    tm.start(limits,pos.sideToMove());

    iterative_deepening();

    if (info.pv.length == 0){
        std::cerr << "WARNING: No PV found after search! Generating emergency move..." << std::endl;
        std::cerr << "Nodes: " << nodes << ", Time: " << tm.elapsed() << "ms" << std::endl;
        return NO_MOVE;
    }
    return info.pv.moves[0];
}

void Searcher::iterative_deepening() {
    Move bestMoveFound = NO_MOVE;

    for (int depth = 1; depth <= limits.depth; ++depth) {
        if (stopFlag) break;

        for(int i=0;i<MAX_PLY;i++){
            stack[i].pv.clear();
        }

        int score;
        if (pos.sideToMove() == White) {
            score = pvs<White, true>(depth, 0, -INFINITE, INFINITE, false);
        } else {
            score = pvs<Black, true>(depth, 0, -INFINITE, INFINITE, false);
        }
        
        // check hard time
        check_time();
        if (stopFlag) break;

        // Add bounds check before copying!
        if (stack[0].pv.length > 0) {
            Move move = stack[0].pv.moves[0];
            if (move.from() != move.to()) { 
                info.pv = stack[0].pv;
                bestMoveFound = stack[0].pv.moves[0];
            }
        }

        //std::cerr << "DEBUG: About to access stack[0].pv\n";
        //std::cerr << "DEBUG: info.pv copied, length=" << info.pv.length << "\n";
        info.depth = depth;
        info.score = score;
        info.nodes = nodes;
        info.time = tm.elapsed();
        info.nps = (info.time > 0) ? (1000ULL * nodes / info.time) : 0ULL;
        //std::cerr << "DEBUG: About to update UCI info\n";
        update_uci_info(depth, score, info.pv);
        //std::cerr << "DEBUG: UCI info updated\n";
    }

    //std::cerr << "DEBUG: About to print bestmove, pv.length=" << info.pv.length << "\n";
    // Output the saved best move
    if (bestMoveFound.from() != bestMoveFound.to()) {
        std::cout << "bestmove " << bestMoveFound.to_uci_string() << std::endl;
    } else {
        std::cout << "bestmove 0000" << std::endl;
    }
}

template <Color c, bool PvNode>
int Searcher::pvs(int depth, int ply, int alpha, int beta, bool cutNode) {

    if (depth <= 0) {
        return quiescence<c>(alpha, beta, ply);
    }

    //clear pvline at this ply
    if(ply<MAX_PLY){
        stack[ply].pv.clear();
    }

    //Initialize LMR table on first call only
    static bool lmrInit=false;
    if(!lmrInit){
        initLMR();
        lmrInit=true;
    }

    nodes++;

    // Stop/time check every 2048 nodes
    if ((nodes & 2047) == 0) {
        check_time();
    }

    if (stopFlag) return alpha;

    if (ply >= MAX_PLY) {
        return eval.evaluate_board(pos);
    }

    // ============= DRAW DETECTION ==========
    if(pos.isDrawByFiftyMove()) return 0;
    if(pos.isDrawByRepetition(ply)) return 0;

    bool inCheck=pos.inCheck<c>();
    
    //for LMR
    bool improving = false;
    if (ply >= 2 && !inCheck) {
        int staticEval = eval.evaluate_board(pos);
        // Store in stack
        stack[ply].staticEval = staticEval;
    
        // Compare to 2 plies ago (same side to move)
        if (ply >= 2) {
            improving = staticEval > stack[ply - 2].staticEval;
        }
    }

    // ============= MATE DISTANCE PRUNING ============
    int mateALPHA=-MATE_SCORE+ply;
    int mateBETA=MATE_SCORE-ply+1;
    alpha=std::max(alpha,mateALPHA);
    beta=std::min(beta,mateBETA);
    if(alpha>=beta) return beta;

    // =============NULL MOVE PRUNIGN =================
    if(!PvNode&&!inCheck){
        int nullScore=0;

        if(tryNullMove<c>(alpha,beta,depth,ply,nullScore)){
            if(depth>=12 && abs(nullScore)<MATE_BOUND){

                int R_local=2+depth/8;
                int staticEval_for_R = eval.evaluate_board(pos);
                if (staticEval_for_R >= beta + 200) R_local++;
                if (depth - R_local - 1 < 1) R_local = depth - 2;

                int verifyScore=pvs<c,false>(depth-R_local,ply,beta-1,beta,false);
                
                if(verifyScore>=beta){
                    return beta;
                }
            }
            else{
                return beta;
            }
        }
    }

    // =========== REVERSE FUTILITY PRUNING ============
    if(!PvNode&&!inCheck){
        int rfpScore=0;
        if(tryReverseFutility<c>(beta,depth,ply,rfpScore)){
            return beta;
        }
    }

    // =================== RAZORING ===================
    if(!inCheck){
        int razoredScore=tryRazoring<c>(alpha,depth,ply);
        if(razoredScore!=NO_RAZOR){
            return razoredScore;
        }
    }


    // Transposition Table probe
    int ttScore;
    Move ttMove;
    if (tt.probe(pos.hash(), depth, alpha, beta, ttScore, ttMove, ply)){
        return ttScore;
    }

    // Move generation
    MoveList moves;
    gen.generate_all_moves<c>(pos, moves);
    if (moves.empty())
        return pos.inCheck<c>() ? -MATE_SCORE + ply : 0;

    orderer.scoreMoves(pos, moves, ttMove, stack[ply].killers);

    Move bestMove = NO_MOVE;
    int bestScore = -INFINITE;
    int legalMoves = 0;
    
    //get static eval for futility pruning
    int staticEval=0;
    bool canDoFutility=!inCheck&&!PvNode&&depth<=3;
    if(canDoFutility){
        staticEval=eval.evaluate_board(pos);
    }

    for (const Move move : moves) {
        pos.makemove<c>(move);

         if (pos.inCheck<c>()) {
            pos.unmakemove<c>(move);
            continue;
        }

        legalMoves++;

        bool givesCheck = pos.inCheck<~c>();
        bool isCapture = move.is_capture();
        bool isPromotion = move.is_promotion();

        // ========== FUTILITY PRUNING ==========
        if (canDoFutility&&legalMoves>1) {
            if (canFutilityPrune<c>(alpha, depth, staticEval, isCapture, isPromotion, givesCheck)) {
                pos.unmakemove<c>(move);
                continue;
            }
        }

        // ========== LATE MOVE PRUNING ==========
        if(legalMoves>1 && shouldPruneMove<c>(depth,legalMoves,inCheck,isCapture,isPromotion,givesCheck)){
            pos.unmakemove<c>(move);
            continue;
        }

        //should we reduce this move
        bool doReduce = shouldReduceMove(depth,legalMoves,inCheck,isCapture,isPromotion)&&(!givesCheck);

        int score;
        if (legalMoves==1) {
            score = -pvs<~c, PvNode>(depth - 1, ply + 1, -beta, -alpha, false);
        } else {
            int reduction=0;
            //if reduction possible
            if(doReduce){
                reduction=getLMRReduction(depth,legalMoves,PvNode,improving);
            }

            score = -pvs<~c, false>(depth - 1 - reduction, ply + 1, -alpha - 1, -alpha, true);

            //if reduce search fails then re-search at full depth
            if(reduction > 0 && score > alpha){
                score=-pvs<~c,PvNode>(depth-1,ply+1,-beta,-alpha,false);
            }

            else if (score > alpha && score < beta){
                score = -pvs<~c, PvNode>(depth - 1, ply + 1, -beta, -alpha, false);
            }
        }

        pos.unmakemove<c>(move);

        if (score > bestScore) {
            bestScore = score;
            bestMove = move;
            if(ply < MAX_PLY && ply >=0 && move.from()!=move.to()){
                stack[ply].pv.update(move, stack[ply + 1].pv);
            }

            if (score > alpha) {
                alpha = score;
                if (score >= beta) {
                    if (!isCapture && ply < MAX_PLY) {
                        stack[ply].killers[1] = stack[ply].killers[0];
                        stack[ply].killers[0] = move;
                    }
                    break; // beta cutoff
                }
            }
        }
    }

    if (legalMoves == 0) {
        return pos.inCheck<c>() ? (-MATE_SCORE + ply) : 0;
    }

    // Store to TT
    int flag = (bestScore >= beta) ? HASH_FLAG_BETA :
                (bestScore > alpha) ? HASH_FLAG_EXACT :
                                      HASH_FLAG_ALPHA;
    tt.store(pos.hash(), depth, flag, bestScore, 0, ply, bestMove);

    return bestScore;
}


template <Color c>
int Searcher::quiescence(int alpha, int beta, int ply) {
    // Hard limit to prevent infinite recursion
    if (ply >= MAX_PLY-1) {
        return eval.evaluate_board(pos);
    }

    nodes++;
    
    // Check time periodically
    if ((nodes & 2047) == 0) {
        check_time();
    }
    if (stopFlag) return 0;

    bool inCheck = pos.inCheck<c>();
    
    // Stand pat (only if not in check)
    int standPat = 0;
    if (!inCheck) {
        standPat = eval.evaluate_board(pos);
        
        if (standPat >= beta) {
            return beta;
        }
        
        if (standPat > alpha) {
            alpha = standPat;
        }
        
        // Delta pruning
        constexpr int DELTA_MARGIN = 1225;
        if (standPat + DELTA_MARGIN < alpha) {
            return alpha;
        }
    }

    MoveList movelist;
    gen.generate_all_moves<c>(pos, movelist);
    if (movelist.empty()) {
        return inCheck ? (-MATE_SCORE + ply) : 0;
    }

    // Draw detection
    if (pos.isDrawByFiftyMove() || pos.isDrawByRepetition(ply)) {
        return 0;
    }

    MoveList interestingMoves;
    interestingMoves.reserve(movelist.size());

    if (inCheck) {
       interestingMoves = movelist;
    } else {
        for (const auto& move : movelist) {
            if(move.is_capture()){
                interestingMoves.push_back(move);
            }
            else if (move.is_promotion()){
                interestingMoves.push_back(move);
            }
        }
    }

    // If no tactical moves and not in check, return stand pat
    if (!inCheck && interestingMoves.empty()) {
        return alpha;
    }

        // Search all interesting moves
    int legalMoves = 0;
    for (const Move& move : interestingMoves) {
        // Make the move
        pos.makemove<c>(move);
        
        // CRITICAL: Check if move is legal (doesn't leave king in check)
        if (pos.inCheck<c>()) {
            pos.unmakemove<c>(move);
            continue;
        }
        
        legalMoves++;

        // Recursive quiescence search with negamax
        int score = -quiescence<~c>(-beta, -alpha, ply + 1);
        
        // Unmake the move
        pos.unmakemove<c>(move);

        // Beta cutoff
        if (score >= beta) {
            return beta;
        }
        
        // Update alpha
        if (score > alpha) {
            alpha = score;
        }
    }

    // If in check and no legal moves found, it's checkmate
    if (inCheck && legalMoves == 0) {
        return -MATE_SCORE + ply;
    }

    return alpha;
}

*/

Searcher::Searcher(Position& pos, TranspositionTable& tt)
    : pos(pos), tt(tt), stopFlag(false), nodes(0), selDepth(0) {

        //std::cerr << "DEBUG: Searcher constructor called\n";
    
    // Initialize stack
    for (int i = 0; i < MAX_PLY + 10; i++) {
        stack[i] = SearchStack();
    }
    
    //std::cerr << "DEBUG: Searcher constructor done\n";
}


Move Searcher::think(const SearchLimits& limits) {
    //std::cerr<<"DEBUG: think() called\n";
    this->limits = limits;
    this->stopFlag = false;
    this->nodes = 0;
    this->info.clear();
    this->startTime = std::chrono::steady_clock::now();
    this->selDepth = 0;

    //std::cerr << "DEBUG: Position side to move: " << (int)pos.sideToMove() << "\n";
    //std::cerr << "DEBUG: Position hash: " << pos.hash() << "\n";
    //std::cerr << "DEBUG: New search\n";
    tt.newSearch();
    //std::cerr << "DEBUG: Starting time manager\n";
    tm.start(limits,pos.sideToMove());

    //std::cerr << "DEBUG: Starting iterative deepening\n";
    iterative_deepening(); // main search loop

    //std::cerr << "DEBUG: After iterative deepening, pv.length=" << info.pv.length << "\n";
    if (info.pv.length == 0)
        return NO_MOVE;
    return info.pv.moves[0];
}

void Searcher::iterative_deepening() {
    Move bestMoveFound = NO_MOVE;

    //std::cerr << "DEBUG: iterative_deepening() entered\n";
    //std::cerr << "DEBUG: limits.depth = " << limits.depth << "\n";

    for (int depth = 1; depth <= limits.depth; ++depth) {
        //std::cerr << "DEBUG: Depth " << depth << "\n";
        if (stopFlag) break;
        //std::cerr << "DEBUG: pvs returned score=" <<  "\n";
        stack[0].pv.clear();
        int score;
        if (pos.sideToMove() == White) {
            score = pvs<White, true>(depth, 0, -INFINITE, INFINITE, false);
        } else {
            score = pvs<Black, true>(depth, 0, -INFINITE, INFINITE, false);
        }
        
        // check hard time
        check_time();
        if (stopFlag) break;

        // Add bounds check before copying!
        if (stack[0].pv.length < MAX_PLY && stack[0].pv.length > 0) {
            info.pv = stack[0].pv;
            bestMoveFound = stack[0].pv.moves[0];
        }

        //std::cerr << "DEBUG: About to access stack[0].pv\n";
        //std::cerr << "DEBUG: info.pv copied, length=" << info.pv.length << "\n";
        info.depth = depth;
        info.score = score;
        info.nodes = nodes;
        info.time = tm.elapsed();
        info.nps = (info.time > 0) ? (1000ULL * nodes / info.time) : 0ULL;
        //std::cerr << "DEBUG: About to update UCI info\n";
        update_uci_info(depth, score, info.pv);
        //std::cerr << "DEBUG: UCI info updated\n";
    }

    //std::cerr << "DEBUG: About to print bestmove, pv.length=" << info.pv.length << "\n";
    // Output the saved best move
    if (bestMoveFound.from() != bestMoveFound.to()) {
        std::cout << "bestmove " << bestMoveFound.to_uci_string() << std::endl;
    } else {
        std::cout << "bestmove 0000" << std::endl;
    }
}

template <Color c, bool PvNode>
int Searcher::pvs(int depth, int ply, int alpha, int beta, bool cutNode) {
    //std::cerr << "DEBUG: pvs entered, depth=" << depth << " ply=" << ply << "\n";
    
    if (ply >= MAX_PLY) {
        return eval.evaluate_board(pos);
    }

    if (depth <= 0) {
        //std::cerr << "DEBUG: Entering quiescence\n";
        return quiescence<c>(alpha, beta, ply);
    }
    nodes++;

    // Stop/time check every 2048 nodes
    if ((nodes & 2047) == 0) check_time();
    if (stopFlag) return 0;
    //std::cerr << "DEBUG: About to probe TT\n";
    // Transposition Table probe
    int ttScore;
    Move ttMove;
    if (tt.probe(pos.hash(), depth, alpha, beta, ttScore, ttMove, ply)){
        //std::cerr << "DEBUG: TT hit, returning " << ttScore << "\n";
        return ttScore;
    }
    //std::cerr << "DEBUG: About to generate moves\n";
    // Move generation
    MoveList moves;
    gen.generate_all_moves<c>(pos, moves);
    //std::cerr << "DEBUG: Generated " << moves.size() << " moves\n";
    if (moves.empty())
        return pos.inCheck<c>() ? -MATE_SCORE + ply : 0;

    //std::cerr << "DEBUG: About to score moves\n";
    orderer.scoreMoves(pos, moves, ttMove, stack[ply].killers);
    //std::cerr << "DEBUG: Moves scored\n";

    Move bestMove = NO_MOVE;
    int bestScore = -INFINITE;
    int legalMoves = 0;
    
    //std::cerr << "DEBUG: Starting move loop\n";
    for (const Move move : moves) {
        //std::cerr << "DEBUG: Trying move " << move.to_uci_string() << "\n";
        pos.makemove<c>(move);

         if (pos.inCheck<c>()) {
            pos.unmakemove<c>(move);
            continue;
        }

        legalMoves++;

        int score;
        if (legalMoves==1) {
            //std::cerr << "DEBUG: First move, full window search\n";
            score = -pvs<~c, PvNode>(depth - 1, ply + 1, -beta, -alpha, false);
        } else {
            //std::cerr << "DEBUG: Zero window search\n";
            score = -pvs<~c, false>(depth - 1, ply + 1, -alpha - 1, -alpha, true);
            if (score > alpha && score < beta){
                //std::cerr << "DEBUG: Re-search with full window\n";
                score = -pvs<~c, PvNode>(depth - 1, ply + 1, -beta, -alpha, false);
            }
        }

        pos.unmakemove<c>(move);
        //std::cerr << "DEBUG: Move unmade, score=" << score << "\n";

        if (score > bestScore) {
            bestScore = score;
            bestMove = move;
            //std::cerr << "DEBUG: About to update PV at ply=" << ply << "\n";
            if(ply < MAX_PLY && ply >=0 ){
                stack[ply].pv.update(move, stack[ply + 1].pv);
            }
            //std::cerr << "DEBUG: PV updated\n";

            if (score > alpha) {
                alpha = score;
                if (score >= beta) {
                    //std::cerr << "DEBUG: Beta cutoff\n";
                    if (!move.is_capture() && ply < MAX_PLY) {
                        stack[ply].killers[1] = stack[ply].killers[0];
                        stack[ply].killers[0] = move;
                    }
                    break; // beta cutoff
                }
            }
        }
    }

    if (legalMoves == 0) {
        return pos.inCheck<c>() ? (-MATE_SCORE + ply) : 0;
    }

    //std::cerr << "DEBUG: Storing to TT\n";
    // Store to TT
    int flag = (bestScore >= beta) ? HASH_FLAG_BETA :
                (bestScore > alpha) ? HASH_FLAG_EXACT :
                                      HASH_FLAG_ALPHA;
    //std::cerr << "DEBUG: pvs returning " << bestScore << "\n";
    tt.store(pos.hash(), depth, flag, bestScore, 0, ply, bestMove);

    return bestScore;
}


template <Color c>
int Searcher::quiescence(int alpha, int beta, int ply) {
    // Hard limit to prevent infinite recursion
    if (ply >= MAX_PLY-1) {
        //std::cerr << "DEBUG: Quiescence depth limit reached!\n";
        return eval.evaluate_board(pos);
    }

    nodes++;
    
    // Check time periodically
    if ((nodes & 2047) == 0) {
        check_time();
    }
    if (stopFlag) return 0;

    bool inCheck = pos.inCheck<c>();
    
    // Stand pat (only if not in check)
    int standPat = 0;
    if (!inCheck) {
        standPat = eval.evaluate_board(pos);
        
        if (standPat >= beta) {
            return beta;
        }
        
        if (standPat > alpha) {
            alpha = standPat;
        }
        
        // Delta pruning
        constexpr int DELTA_MARGIN = 1225;
        if (standPat + DELTA_MARGIN < alpha) {
            return alpha;
        }
    }

    MoveList movelist;
    //std::cerr << "DEBUG: About to generate moves in qsearch\n";
    gen.generate_all_moves<c>(pos, movelist);
    //std::cerr << "DEBUG: Generated " << movelist.size() << " moves in qsearch\n";
    if (movelist.empty()) {
        //std::cerr << "DEBUG: No moves, returning mate/stalemate score\n";
        return inCheck ? (-MATE_SCORE + ply) : 0;
    }

    // Draw detection
    if (pos.isDrawByFiftyMove() || pos.isDrawByRepetition(ply)) {
        return 0;
    }

    MoveList interestingMoves;
    interestingMoves.reserve(movelist.size());

    if (inCheck) {
        //std::cerr << "DEBUG: In check, considering all moves\n";
       interestingMoves = movelist;
    } else {
        //std::cerr << "DEBUG: Not in check, filtering captures/promotions\n";
        for (const auto& move : movelist) {
            if(move.is_capture()){
                interestingMoves.push_back(move);
            }
            else if (move.is_promotion()){
                interestingMoves.push_back(move);
            }
        }
    }

    // If no tactical moves and not in check, return stand pat
    if (!inCheck && interestingMoves.empty()) {
        return alpha;
    }

        // Search all interesting moves
    int legalMoves = 0;
    for (const Move& move : interestingMoves) {
        // Make the move
        pos.makemove<c>(move);
        
        // CRITICAL: Check if move is legal (doesn't leave king in check)
        if (pos.inCheck<c>()) {
            pos.unmakemove<c>(move);
            continue;
        }
        
        legalMoves++;

        // Recursive quiescence search with negamax framework
        int score = -quiescence<~c>(-beta, -alpha, ply + 1);
        
        // Unmake the move
        pos.unmakemove<c>(move);

        // Beta cutoff
        if (score >= beta) {
            return beta;
        }
        
        // Update alpha
        if (score > alpha) {
            alpha = score;
        }
    }

    // If in check and no legal moves found, it's checkmate
    if (inCheck && legalMoves == 0) {
        return -MATE_SCORE + ply;
    }

    return alpha;
}
void Searcher::check_time() {
    //checks if hardTime exceeded
    tm.Check();

    //stops when hard time reached
    if (tm.StopFlag()){
        stopFlag = true;
    }
}

void Searcher::update_uci_info(int depth, int score, const PVLine& pv) {
    std::cout << "info depth " << depth
              << " score cp " << score
              << " nodes " << nodes
              << " nps " << info.nps
              << " time " << info.time
              << " pv ";
    int maxMoves = std::min(pv.length, MAX_PLY);
    for (int i = 0; i < maxMoves; i++){
        std::cout << pv.moves[i].to_uci_string() << " ";
    }
    std::cout << std::endl;
}

bool Searcher::is_draw(int ply) const {
    return pos.isDrawByRepetition(ply) || pos.isDrawByFiftyMove();
}

void Searcher::newGame() {
    nodes = 0;
    selDepth = 0;
    stopFlag = false;
    for (int i = 0; i < MAX_PLY + 10; i++) {
        stack[i].clear();
    }
}

};

/*
int Searcher::score_to_tt(int score, int ply) const {
    if (score >= MATE_BOUND) {
        return score + ply;  // Convert mate-in-N to absolute mate score
    }
    if (score <= -MATE_BOUND) {
        return score - ply;
    }
    return score;
}

int Searcher::score_from_tt(int score, int ply) const {
    if (score >= MATE_BOUND) {
        return score - ply;  // Convert back to mate-in-N
    }
    if (score <= -MATE_BOUND) {
        return score + ply;
    }
    return score;
}
*/
