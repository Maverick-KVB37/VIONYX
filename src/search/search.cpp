#include "search.h"
#include <iostream>

namespace Search {

Searcher::Searcher(Position& pos, TranspositionTable& tt)
    : pos(pos), tt(tt), stopFlag(false), nodes(0), selDepth(0) {
    
    // Initialize stack
    for (int i = 0; i < MAX_PLY + 10; i++) {
        stack[i] = SearchStack();
    }
    
}


Move Searcher::think(const SearchLimits& limits) {
    this->limits = limits;
    this->stopFlag = false;
    this->nodes = 0;
    this->info.clear();
    this->startTime = std::chrono::steady_clock::now();
    this->selDepth = 0;

    tt.newSearch();

    //calculate move number
    int fullMoves=pos.getFullMoves();
    int moveNumber;
    if(pos.sideToMove()==White){
        moveNumber=(fullMoves-1)*2;
    }
    else{
        moveNumber=(fullMoves-1)*2+1;
    }

    // Debug output
    /*std::cerr << "DEBUG: fullMoves=" << fullMoves 
              << " side=" << (pos.sideToMove() == White ? "White" : "Black")
              << " moveNumber=" << moveNumber << std::endl;
    */
    tm.start(limits,pos.sideToMove(),moveNumber);

    // Debug output
    //std::cerr << "DEBUG: Allocated time: " << tm.allocatedTime() << "ms" << std::endl;

    iterative_deepening(); // main search loop

    Move bestMove = NO_MOVE;
    if (info.pv.length > 0) {
        bestMove = info.pv.moves[0];
    }
    
    bool moveInvalid = (bestMove.from() == bestMove.to()) || 
                       (bestMove.from() >= 64) || 
                       (bestMove.to() >= 64);
    
    if (info.pv.length == 0 || moveInvalid) {
        std::cerr << "WARNING: Invalid PV move, generating emergency move" << std::endl;
        
        // Generate emergency move
        MoveList moves;
        if (pos.sideToMove() == White) {
            gen.generate_all_moves<White>(pos, moves);
            
            for (const Move& move : moves) {
                pos.makemove<White>(move);
                bool legal = !pos.inCheck<White>();
                pos.unmakemove<White>(move);
                
                if (legal) {
                    std::cerr << "Emergency move selected: " << move.to_uci_string() << std::endl;
                    return move;
                }
            }
        } else {
            gen.generate_all_moves<Black>(pos, moves);
            
            for (const Move& move : moves) {
                pos.makemove<Black>(move);
                bool legal = !pos.inCheck<Black>();
                pos.unmakemove<Black>(move);
                
                if (legal) {
                    std::cerr << "Emergency move selected: " << move.to_uci_string() << std::endl;
                    return move;
                }
            }
        }
        
        // Last resort: return first move if any exist
        if (!moves.empty()) {
            std::cerr << "CRITICAL: Returning first pseudo-legal move!" << std::endl;
            return moves[0];
        }
        
        std::cerr << "FATAL: No moves available!" << std::endl;
        return NO_MOVE;
    }
    
    return bestMove;
}

void Searcher::iterative_deepening() {
    Move bestMoveFound = NO_MOVE;

    for (int depth = 1; depth <= limits.depth; ++depth) {
        if (stopFlag && depth>1) break;
        stack[0].pv.clear();
        int score;
        if (pos.sideToMove() == White) {
            score = pvs<White, true>(depth, 0, -INFINITE, INFINITE, false);
        } else {
            score = pvs<Black, true>(depth, 0, -INFINITE, INFINITE, false);
        }
        
        // check hard time
        if(depth>1){
            check_time();
            if (stopFlag) break;
        }

        // Add bounds check before copying!
        if (stack[0].pv.length < MAX_PLY && stack[0].pv.length > 0) {
            info.pv = stack[0].pv;
            bestMoveFound = stack[0].pv.moves[0];
        }

        info.depth = depth;
        info.score = score;
        info.nodes = nodes;
        info.time = tm.elapsed();
        info.nps = (info.time > 0) ? (1000ULL * nodes / info.time) : 0ULL;
        update_uci_info(depth, score, info.pv);
    }

    // Output the saved best move
    if (bestMoveFound.from() != bestMoveFound.to()) {
        std::cout << "bestmove " << bestMoveFound.to_uci_string() << std::endl;
    } else {
        std::cout << "bestmove 0000" << std::endl;
    }
}

template <Color c, bool PvNode>
int Searcher::pvs(int depth, int ply, int alpha, int beta, bool cutNode) {
    
    if (ply >= MAX_PLY) {
        return eval.evaluate_board(pos);
    }

    if (depth <= 0) {
        return quiescence<c>(alpha, beta, ply);
    }
    nodes++;

    // Stop/time check every 2048 nodes
    if ((nodes & 2047) == 0) check_time();
    if (stopFlag) return 0;
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
    
    for (const Move move : moves) {
        pos.makemove<c>(move);

         if (pos.inCheck<c>()) {
            pos.unmakemove<c>(move);
            continue;
        }

        legalMoves++;

        int score;
        if (legalMoves==1) {
            score = -pvs<~c, PvNode>(depth - 1, ply + 1, -beta, -alpha, false);
        } else {
            score = -pvs<~c, false>(depth - 1, ply + 1, -alpha - 1, -alpha, true);
            if (score > alpha && score < beta){
                score = -pvs<~c, PvNode>(depth - 1, ply + 1, -beta, -alpha, false);
            }
        }

        pos.unmakemove<c>(move);

        if (score > bestScore) {
            bestScore = score;
            bestMove = move;
            if(ply < MAX_PLY && ply >=0 ){
                stack[ply].pv.update(move, stack[ply + 1].pv);
            }

            if (score > alpha) {
                alpha = score;
                if (score >= beta) {
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

