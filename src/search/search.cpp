#include "search.h"
#include <iostream>

namespace Search {

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

