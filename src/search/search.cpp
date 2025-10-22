#include "search.h"
#include <iostream>

namespace Search {

Searcher::Searcher(Position& pos, TranspositionTable& tt)
    : pos(pos), tt(tt), stopFlag(false), nodes(0), selDepth(0) {}

Move Searcher::think(const SearchLimits& limits) {
    this->limits = limits;
    this->stopFlag = false;
    this->nodes = 0;
    this->info.clear();
    this->startTime = std::chrono::steady_clock::now();
    this->selDepth = 0;

    tt.newSearch();
    tm.start(limits);

    iterative_deepening(); // main search loop

    if (info.pv.length == 0)
        return NO_MOVE;
    return info.pv.moves[0];
}

void Searcher::iterative_deepening() {

    for (int depth = 1; depth <= limits.depth; ++depth) {
        if (stopFlag) break;

        int score = pvs<White, true>(depth, 0, -INFINITE, INFINITE, false);

        // Time & cutoff check
        check_time();
        if (stopFlag) break;

        // Update PV line after search
        info.pv = stack[0].pv;
        info.depth = depth;
        info.score = score;
        info.nodes = nodes;
        info.time = tm.elapsed();
        info.nps = (info.time > 0) ? (1000ULL * nodes / info.time) : 0ULL;

        update_uci_info(depth, score, info.pv);
    }

    std::cout << "bestmove " << (info.pv.moves[0]).to_uci_string() << std::endl;
}

template <Color c, bool PvNode>
int Searcher::pvs(int depth, int ply, int alpha, int beta, bool cutNode) {
    if (depth <= 0) return quiescence<c>(alpha, beta, ply);
    nodes++;

    // Stop/time check every 2048 nodes
    if ((nodes & 2047) == 0) check_time();
    if (stopFlag) return 0;

    // Transposition Table probe
    int ttScore;
    Move ttMove;
    if (tt.probe(pos.hash(), depth, alpha, beta, ttScore, ttMove, ply))
        return ttScore;

    // Move generation
    MoveList moves;
    this->gen.generate_all_moves<c>(pos, moves);
    if (moves.empty())
        return pos.inCheck<c>() ? -MATE_SCORE + ply : 0;

    orderer.scoreMoves(pos, moves, ttMove, stack[ply].killers);

    Move bestMove = NO_MOVE;
    int bestScore = -INFINITE;
    bool firstMove = true;

    for (const Move move : moves) {
        pos.makemove<c>(move);
        if (pos.inCheck<~c>()) {
            pos.unmakemove<c>(move);
            continue;
        }

        int score;
        if (firstMove) {
            firstMove = false;
            score = -pvs<~c, PvNode>(depth - 1, ply + 1, -beta, -alpha, false);
        } else {
            score = -pvs<~c, false>(depth - 1, ply + 1, -alpha - 1, -alpha, true);
            if (score > alpha && score < beta)
                score = -pvs<~c, PvNode>(depth - 1, ply + 1, -beta, -alpha, false);
        }

        pos.unmakemove<c>(move);

        if (score > bestScore) {
            bestScore = score;
            bestMove = move;
            stack[ply].pv.update(move, stack[ply + 1].pv);
            if (score > alpha) {
                alpha = score;
                if (score >= beta) break; // beta cutoff
            }
        }
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
    bool inCheck = pos.inCheck<c>();
    MoveList movelist;
    gen.generate_all_moves<c>(pos, movelist);

    if (movelist.empty()) {
        return inCheck ? (-MATE_SCORE + ply) : 0;
    }

    if (pos.isDrawByFiftyMove() || pos.isDrawByRepetition(ply)) {
        return 0;
    }

    if ((nodes & 2047) == 0) {
        tm.Check();  // Fixed from check_time
    }

    if (stopFlag.load(std::memory_order_relaxed) || tm.StopFlag()) {
        return 0;
    }

    int stand_pat = eval.evaluate_board(pos);
    if (ply >= MAX_PLY - 1) {
        return stand_pat;
    }

    if (!inCheck) {
        if (stand_pat >= beta) return beta;
        constexpr int DELTA_MARGIN = 1225;
        if (stand_pat + DELTA_MARGIN < alpha) return alpha;
        if (stand_pat > alpha) alpha = stand_pat;
    }

    struct ScoredMove {
        Move move;
        int score;
    };
    std::vector<ScoredMove> interestingMoves;
    interestingMoves.reserve(movelist.size());

    if (inCheck) {
        for (const auto& move : movelist) {
            interestingMoves.push_back({move, 0});
        }
    } else {
        for (const auto& move : movelist) {
            bool isCapture = (pos.pieceAt(move.to()) != None) ||
                (piecetype(pos.pieceAt(move.from())) == Pawn &&
                 move.to() == pos.epSquare());  // fixed en passant accessor
            bool isPromotion = move.is_promotion();

            int score = 0;
            if (isPromotion) {
                if (isCapture) {
                    score = 11000 + orderer.see(pos, move);
                } else {
                    score = 10000;
                }
                interestingMoves.push_back({move, score});
            } else if (isCapture) {
                if (orderer.seeGe(pos, move, -100)) {
                    score = 9000 + orderer.see(pos, move);
                    interestingMoves.push_back({move, score});
                }
            }
        }
    }

    std::sort(interestingMoves.begin(), interestingMoves.end(),
        [](const ScoredMove& a, const ScoredMove& b) {
            return a.score > b.score;
        });

    for (const auto& smove : interestingMoves) {
        pos.makemove<c>(smove.move);
        int score = -quiescence<~c>(-beta, -alpha, ply + 1);
        pos.unmakemove<c>(smove.move);

        if (score >= beta) return beta;
        if (score > alpha) alpha = score;
    }

    return alpha;
}


void Searcher::check_time() {
    if (tm.elapsed() >= tm.softTime() || tm.StopFlag())
        stopFlag = true;
}

void Searcher::update_uci_info(int depth, int score, const PVLine& pv) {
    std::cout << "info depth " << depth
              << " score cp " << score
              << " nodes " << nodes
              << " nps " << info.nps
              << " time " << info.time
              << " pv ";
    for (int i = 0; i < pv.length; ++i)
        std::cout << pv.moves[i].to_uci_string() << " ";
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

