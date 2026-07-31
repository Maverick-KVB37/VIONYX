#include "search.h"
#include <iostream>

namespace Search {

Searcher::Searcher(Position &pos, TranspositionTable &tt)
    : pos(pos), tt(tt), stopFlag(false), nodes(0), selDepth(0) {

  for (int i = 0; i < MAX_PLY + 10; i++) {
    stack[i] = SearchStack();
  }

  initLmrTable();
}

void Searcher::initLmrTable() {
  lmrTable[0][0] = 0;
  for (int d = 1; d < 64; d++)
    for (int m = 1; m < 64; m++)
      lmrTable[d][m] = int(0.75 + log(d) * log(m) / 2.25);
}

// age history scores (halve) before each search
void Searcher::ageHistory() {
  for (int c = 0; c < 2; c++)
    for (int f = 0; f < 64; f++)
      for (int t = 0; t < 64; t++)
        history[c][f][t] /= 2;
}

Move Searcher::think(const SearchLimits &limits) {
  this->limits = limits;
  this->stopFlag = false;
  this->nodes = 0;
  this->info.clear();
  this->startTime = std::chrono::steady_clock::now();
  this->selDepth = 0;

  tt.newSearch();

  int fullMoves = pos.getFullMoves();
  int moveNumber;
  if (pos.sideToMove() == White) {
    moveNumber = (fullMoves - 1) * 2;
  } else {
    moveNumber = (fullMoves - 1) * 2 + 1;
  }

  tm.start(limits, pos.sideToMove(), moveNumber);

  IterativeDeepening();

  Move bestMove = NO_MOVE;
  if (info.pv.length > 0) {
    bestMove = info.pv.moves[0];
  }

  bool moveInvalid = (bestMove.from() == bestMove.to()) ||
                     (bestMove.from() >= 64) || (bestMove.to() >= 64);

  if (info.pv.length == 0 || moveInvalid) {

    MoveList moves;
    if (pos.sideToMove() == White) {
      gen.GenerateAllMoves<White>(pos, moves);

      for (const Move &move : moves) {
        pos.makemove<White>(move);
        bool legal = !pos.inCheck<White>();
        pos.unmakemove<White>(move);

        if (legal) {
          return move;
        }
      }
    } else {
      gen.GenerateAllMoves<Black>(pos, moves);

      for (const Move &move : moves) {
        pos.makemove<Black>(move);
        bool legal = !pos.inCheck<Black>();
        pos.unmakemove<Black>(move);

        if (legal) {
          return move;
        }
      }
    }

    if (!moves.empty()) {
      return moves[0];
    }

    return NO_MOVE;
  }

  return bestMove;
}

void Searcher::IterativeDeepening() {
  Move bestMoveFound = NO_MOVE;
  int score = 0;

  ageHistory();

  for (int depth = 1; depth <= limits.depth; ++depth) {
    if (stopFlag && depth > 1)
      break;

    // aspiration window
    int alpha = -INFINITE;
    int beta = INFINITE;
    int delta = 25;

    if (depth > 4) {
      alpha = std::max(-INFINITE, score - delta);
      beta = std::min(INFINITE, score + delta);
    }
    while (true) {
      stack[0].pv.clear();

      if (pos.sideToMove() == White) {
        score = pvs<White, true>(depth, 0, alpha, beta, false, NO_MOVE);
      } else {
        score = pvs<Black, true>(depth, 0, alpha, beta, false, NO_MOVE);
      }

      if (depth > 1) {
        CheckTime();
        if (stopFlag)
          break;
      }

      // exponential widening on fail-low
      if (score <= alpha) {
        beta = (alpha + beta) / 2;
        alpha = std::max(-INFINITE, alpha - delta);
        delta *= 2;
        continue;
      }
      // exponential widening on fail-high
      if (score >= beta) {
        beta = std::min(INFINITE, beta + delta);
        delta *= 2;
        continue;
      }
      break;
    }

    if (stopFlag)
      break;

    if (stack[0].pv.length < MAX_PLY && stack[0].pv.length > 0) {
      info.pv = stack[0].pv;
      bestMoveFound = stack[0].pv.moves[0];
    }

    info.depth = depth;
    info.score = score;
    info.nodes = nodes;
    info.time = tm.elapsed();
    info.nps = (info.time > 0) ? (1000ULL * nodes / info.time) : 0ULL;
    UpdateUciInfo(depth, score, info.pv);
  }

  if (bestMoveFound.from() != bestMoveFound.to()) {
    std::cout << "bestmove " << bestMoveFound.ToUciString() << std::endl;
  } else {
    std::cout << "bestmove 0000" << std::endl;
  }
}

template <Color c, bool PvNode>
int Searcher::pvs(int depth, int ply, int alpha, int beta, bool cutNode,
                  Move previousMove) {
  if (ply < MAX_PLY) {
    stack[ply].pv.length = 0;
  }

  if (ply >= MAX_PLY) {
    return eval.EvaluateBoard(pos);
  }

  if (depth <= 0) {
    return quiescence<c>(alpha, beta, ply);
  }
  nodes++;

  if (ply > 0 && IsDraw(ply))
    return 0;

  if ((nodes & 2047) == 0)
    CheckTime();
  if (stopFlag)
    return 0;

  // TT probe
  int ttScore;
  Move ttMove = NO_MOVE;
  int ttDepth = 0, ttFlag = 0;
  bool ttHit = tt.probeForSE(pos.hash(), ttDepth, ttFlag, ttScore, ttMove, ply);

  if (ttHit && ttDepth >= depth && !PvNode) {
    if (ttFlag == HASH_FLAG_EXACT)
      return ttScore;
    if (ttFlag == HASH_FLAG_ALPHA && ttScore <= alpha)
      return ttScore;
    if (ttFlag == HASH_FLAG_BETA && ttScore >= beta)
      return ttScore;
  }

  bool inCheck = pos.inCheck<c>();
  int extension = 0;
  if (inCheck) {
    extension = 1;
  }

  // IID
  if (PvNode && depth > 3 && ttMove == NO_MOVE && !inCheck) {
    int iidDepth = depth - 2;
    pvs<c, PvNode>(iidDepth, ply, alpha, beta, false, previousMove);

    Move iidMove = NO_MOVE;
    int iidScore = 0;
    if (tt.probe(pos.hash(), depth, alpha, beta, iidScore, iidMove, ply)) {
      ttMove = iidMove;
    }
  }

  int staticEval = 0;
  if (!inCheck) {
    staticEval = eval.EvaluateBoard(pos);
    stack[ply].staticEval = staticEval;
  }

  bool improving =
      (!inCheck && ply >= 2 && staticEval > stack[ply - 2].staticEval);

  // RFP (reverse futility pruning)
  if (!PvNode && !inCheck && depth <= 8 && pos.hasNonPawnMaterial<c>() &&
      abs(beta) < MATE_SCORE - 100) {
    int margin = improving ? 120 * depth : 80 * depth;
    if (staticEval - margin >= beta) {
      return beta;
    }
  }

  // NMP (null move pruning)
  if (!PvNode && depth >= 3 && !inCheck && ply > 0 && staticEval >= beta &&
      pos.hasNonPawnMaterial<c>()) {
    int R = 3 + (depth / 6) + improving;
    if (staticEval >= beta + 200)
      R++;

    if (depth - R - 1 >= 0) {
      pos.makeNullMove<c>();

      int score = -pvs<~c, false>(depth - R - 1, ply + 1, -beta, -beta + 1,
                                  !cutNode, NO_MOVE);

      pos.unmakeNullMove<c>();
      if (stopFlag)
        return 0;
      if (score >= beta) {
        if (score > MATE_SCORE - MAX_PLY) {
          score = beta;
        }
        // Verification search at high depth to prevent null move zugzwang
        if (depth >= 12) {
          int verScore = pvs<c, false>(depth - R - 1, ply + 1, beta - 1, beta,
                                       false, previousMove);
          if (verScore >= beta)
            return score;
        } else {
          return score;
        }
      }
    }
  }

  // futility pruning
  bool futilityprun = false;
  if (depth <= 4 && !inCheck && !PvNode && std::abs(alpha) < MATE_SCORE - 100 &&
      std::abs(beta) < MATE_SCORE - 100) {
    int margin = depth * 150;
    if (staticEval + margin <= alpha) {
      futilityprun = true;
    }
  }

  MoveList moves;
  gen.GenerateAllMoves<c>(pos, moves);
  if (moves.empty())
    return pos.inCheck<c>() ? -MATE_SCORE + ply : 0;

  orderer.ScoreMoves(pos, moves, ttMove, stack[ply].killers, history,
                     previousMove, counterMoves);

  int originalAlpha = alpha;
  Move bestMove = NO_MOVE;
  int bestScore = -INFINITE;
  int legalMoves = 0;

  int quietmovesearched = 0;

  Move searchedQuiets[64];
  int searchedQuietCount = 0;

  for (int i = 0; i < moves.size(); ++i) {
    Move move = orderer.PickNextMove(moves, i);
    if (move == stack[ply].excludedMove)
      continue;

    // LMP
    if (!PvNode && !inCheck && depth < 8 && !move.IsCapture() &&
        !move.IsPromotion()) {
      int lmpthre = improving ? (3 + depth * depth) : (3 + depth * depth / 2);
      if (quietmovesearched >= lmpthre) {
        continue;
      }
    }

    pos.makemove<c>(move);

    if (pos.inCheck<c>()) {
      pos.unmakemove<c>(move);
      continue;
    }

    legalMoves++;

    if (!move.IsCapture() && !move.IsPromotion()) {
      quietmovesearched++;
      if (searchedQuietCount < 64) {
        searchedQuiets[searchedQuietCount++] = move;
      }
    }

    bool givesCheck = pos.inCheck<~c>();
    if (futilityprun && legalMoves > 1 && !move.IsCapture() &&
        !move.IsPromotion() && !givesCheck) {
      pos.unmakemove<c>(move);
      continue;
    }

    int score;
    bool needfullsearch = true;

    // LMR (Late Move Reductions)
    if (depth >= 3 && legalMoves > 3 && !inCheck && !move.IsCapture() &&
        !move.IsPromotion() && !givesCheck) {

      int R = lmrTable[std::min(depth, 63)][std::min(legalMoves, 63)];

      int historyScore = history[c][move.from()][move.to()];
      if (!PvNode)
        R += 1;
      if (cutNode)
        R += 1;
      if (historyScore < 0)
        R += 1;
      if (historyScore > 4000)
        R -= 1;
      if (!improving)
        R += 1;
      if (PvNode)
        R -= 1;

      R = std::clamp(R, 0, depth - 2);

      int reduceddepth = std::max(1, depth - 1 - R);

      score = -pvs<~c, false>(reduceddepth, ply + 1, -alpha - 1, -alpha, true,
                              move);
      if (score > alpha) {
        needfullsearch = true;
      } else {
        needfullsearch = false;
      }
    }

    if (needfullsearch) {
      if (legalMoves == 1) {
        score = -pvs<~c, PvNode>(depth - 1 + extension, ply + 1, -beta, -alpha,
                                 false, move);
      } else {
        score = -pvs<~c, false>(depth - 1 + extension, ply + 1, -alpha - 1,
                                -alpha, true, move);
        if (score > alpha && score < beta) {
          score = -pvs<~c, PvNode>(depth - 1 + extension, ply + 1, -beta,
                                   -alpha, false, move);
        }
      }
    }

    pos.unmakemove<c>(move);

    if (score > bestScore) {
      bestScore = score;
      bestMove = move;
      if (ply < MAX_PLY && ply >= 0) {
        stack[ply].pv.update(move, stack[ply + 1].pv);
      }

      if (score > alpha) {
        alpha = score;
        if (score >= beta) {
          if (!move.IsCapture() && ply < MAX_PLY) {
            stack[ply].killers[1] = stack[ply].killers[0];
            stack[ply].killers[0] = move;

            // gravity based history update
            int bonus = std::min(depth * depth, 400);
            int side = (c == White) ? 0 : 1;

            history[side][move.from()][move.to()] +=
                bonus -
                history[side][move.from()][move.to()] * std::abs(bonus) / 16384;

            // malus for searched quiets that didn't cut
            for (int qi = 0; qi < searchedQuietCount - 1; qi++) {
              Move bad = searchedQuiets[qi];
              history[side][bad.from()][bad.to()] -=
                  bonus -
                  history[side][bad.from()][bad.to()] * std::abs(bonus) / 16384;
            }

            if (previousMove != NO_MOVE) {
              counterMoves[side][previousMove.from()][previousMove.to()] = move;
            }
          }
          break; // beta cutoff
        }
      }
    }
  }

  if (legalMoves == 0) {
    return pos.inCheck<c>() ? (-MATE_SCORE + ply) : 0;
  }

  // store to TT
  int flag = (bestScore >= beta)           ? HASH_FLAG_BETA
             : (bestScore > originalAlpha) ? HASH_FLAG_EXACT
                                           : HASH_FLAG_ALPHA;
  tt.store(pos.hash(), depth, flag, bestScore, 0, ply, bestMove);

  return bestScore;
}

template <Color c> int Searcher::quiescence(int alpha, int beta, int ply) {
  if (ply >= MAX_PLY - 1) {
    return eval.EvaluateBoard(pos);
  }

  nodes++;

  if ((nodes & 2047) == 0) {
    CheckTime();
  }
  if (stopFlag)
    return 0;

  bool inCheck = pos.inCheck<c>();

  // stand pat
  int standPat = -INFINITE;
  if (!inCheck) {
    standPat = eval.EvaluateBoard(pos);

    if (standPat >= beta) {
      return beta;
    }

    if (standPat > alpha) {
      alpha = standPat;
    }

    // delta pruning
    constexpr int DELTA_MARGIN = 1225;
    if (standPat + DELTA_MARGIN < alpha) {
      return alpha;
    }
  }

  // generate all moves when in check, otherwise only captures+promotions
  MoveList movelist;
  if (inCheck) {
    gen.GenerateAllMoves<c>(pos, movelist);
  } else {
    gen.GenerateCaptures<c>(pos, movelist);
  }

  if (movelist.empty()) {
    return inCheck ? (-MATE_SCORE + ply) : alpha;
  }

  if (inCheck) {
    Move ttMove = NO_MOVE;
    orderer.ScoreMoves(pos, movelist, ttMove, stack[ply].killers, history,
                       NO_MOVE, counterMoves);
  } else {
    orderer.ScoreCaptures(pos, movelist);
  }

  int legalMoves = 0;
  for (int i = 0; i < movelist.size(); ++i) {
    Move move = orderer.PickNextMove(movelist, i);
    pos.makemove<c>(move);

    if (pos.inCheck<c>()) {
      pos.unmakemove<c>(move);
      continue;
    }

    legalMoves++;

    int score = -quiescence<~c>(-beta, -alpha, ply + 1);

    pos.unmakemove<c>(move);

    if (score >= beta) {
      return beta;
    }

    if (score > alpha) {
      alpha = score;
    }
  }

  if (inCheck && legalMoves == 0) {
    return -MATE_SCORE + ply;
  }

  return alpha;
}

void Searcher::CheckTime() {
  tm.Check();

  if (tm.StopFlag()) {
    stopFlag = true;
  }
}

void Searcher::UpdateUciInfo(int depth, int score, const PVLine &pv) {
  std::cout << "info depth " << depth;

  if (std::abs(score) >= MATE_BOUND) {
    int mate_in_plies = MATE_SCORE - std::abs(score);
    int mate_in_moves = (mate_in_plies + 1) / 2;
    std::cout << " score mate " << (score > 0 ? mate_in_moves : -mate_in_moves);
  } else {
    std::cout << " score cp " << score;
  }

  std::cout << " nodes " << nodes << " nps " << info.nps << " time "
            << info.time << " pv ";
  int maxMoves = std::min(pv.length, MAX_PLY);
  for (int i = 0; i < maxMoves; i++) {
    std::cout << pv.moves[i].ToUciString() << " ";
  }
  std::cout << std::endl;
}

bool Searcher::IsDraw(int ply) const {
  return pos.isDrawByRepetition(ply) || pos.isDrawByFiftyMove();
}

void Searcher::newGame() {
  nodes = 0;
  selDepth = 0;
  stopFlag = false;
  clearHistory();
  for (int i = 0; i < MAX_PLY + 10; i++) {
    stack[i].clear();
  }
}

}; // namespace Search
