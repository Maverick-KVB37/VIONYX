#include "search.h"
#include <iostream>

namespace Search {

Searcher::Searcher(Position &pos, TranspositionTable &tt)
    : pos(pos), tt(tt), stopFlag(false), nodes(0), selDepth(0) {

  // Initialize stack
  for (int i = 0; i < MAX_PLY + 10; i++) {
    stack[i] = SearchStack();
  }

  // Pre compute LMR table
  initLmrTable();
}

void Searcher::initLmrTable() {
  lmrTable[0][0] = 0;
  for (int d = 1; d < 64; d++)
    for (int m = 1; m < 64; m++)
      lmrTable[d][m] = int(0.75 + log(d) * log(m) / 2.25);
}

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

  // calculate move number
  int fullMoves = pos.getFullMoves();
  int moveNumber;
  if (pos.sideToMove() == White) {
    moveNumber = (fullMoves - 1) * 2;
  } else {
    moveNumber = (fullMoves - 1) * 2 + 1;
  }

  tm.start(limits, pos.sideToMove(), moveNumber);

  iterative_deepening(); // main search loop

  Move bestMove = NO_MOVE;
  if (info.pv.length > 0) {
    bestMove = info.pv.moves[0];
  }

  bool moveInvalid = (bestMove.from() == bestMove.to()) ||
                     (bestMove.from() >= 64) || (bestMove.to() >= 64);

  if (info.pv.length == 0 || moveInvalid) {

    // Generate emergency move
    MoveList moves;
    if (pos.sideToMove() == White) {
      gen.generate_all_moves<White>(pos, moves);

      for (const Move &move : moves) {
        pos.makemove<White>(move);
        bool legal = !pos.inCheck<White>();
        pos.unmakemove<White>(move);

        if (legal) {
          return move;
        }
      }
    } else {
      gen.generate_all_moves<Black>(pos, moves);

      for (const Move &move : moves) {
        pos.makemove<Black>(move);
        bool legal = !pos.inCheck<Black>();
        pos.unmakemove<Black>(move);

        if (legal) {
          return move;
        }
      }
    }

    // Last resort: return first move if any exist
    if (!moves.empty()) {
      return moves[0];
    }

    return NO_MOVE;
  }

  return bestMove;
}

void Searcher::iterative_deepening() {
  Move bestMoveFound = NO_MOVE;
  int score = 0;

  // Age history scores before each search
  ageHistory();

  for (int depth = 1; depth <= limits.depth; ++depth) {
    if (stopFlag && depth > 1)
      break;

    // ASPIRATION WINDOW with exponential widening
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

      // check hard time
      if (depth > 1) {
        check_time();
        if (stopFlag)
          break;
      }

      // Exponential widening on fail-low
      if (score <= alpha) {
        beta = (alpha + beta) / 2;
        alpha = std::max(-INFINITE, alpha - delta);
        delta *= 2;
        continue;
      }
      // Exponential widening on fail-high
      if (score >= beta) {
        beta = std::min(INFINITE, beta + delta);
        delta *= 2;
        continue;
      }
      break;
    }

    if (stopFlag)
      break;

    // update pv and output it we finish search properly
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
int Searcher::pvs(int depth, int ply, int alpha, int beta, bool cutNode,
                  Move previousMove) {
  if (ply < MAX_PLY) {
    stack[ply].pv.length = 0;
  }

  if (ply >= MAX_PLY) {
    return eval.evaluate_board(pos);
  }

  if (depth <= 0) {
    return quiescence<c>(alpha, beta, ply);
  }
  nodes++;

  // Draw detection (repetition and fifty-move rule)
  if (ply > 0 && is_draw(ply))
    return 0;

  // Stop/time check every 2048 nodes
  if ((nodes & 2047) == 0)
    check_time();
  if (stopFlag)
    return 0;
  // Transposition Table probe
  int ttScore;
  Move ttMove = NO_MOVE;
  if (tt.probe(pos.hash(), depth, alpha, beta, ttScore, ttMove, ply)) {
    // we onlt use tt score to cut off when we are not in a pv node
    if (!PvNode) {
      return ttScore;
    }
  }

  bool inCheck = pos.inCheck<c>();
  // check extension
  int extension = 0;
  if (inCheck) {
    extension = 1;
  }

  // IID(INTERNAL ITERATIVE DEEPENING)
  if (PvNode && depth > 3 && ttMove == NO_MOVE && !inCheck) {
    // so search shallow to find move
    int iidDepth = depth - 2;
    pvs<c, PvNode>(iidDepth, ply, alpha, beta, false, previousMove);

    Move iidMove = NO_MOVE;
    int iidScore = 0;
    if (tt.probe(pos.hash(), depth, alpha, beta, iidScore, iidMove, ply)) {
      // so we found move and can use it as ordering
      ttMove = iidMove;
    }
  }
  // static evaluation
  int staticEval = 0;
  if (!inCheck) {
    staticEval = eval.evaluate_board(pos);
    stack[ply].staticEval = staticEval;
  }

  // 1.3: Improving flag — is our eval better than 2 plies ago?
  bool improving =
      (!inCheck && ply >= 2 && staticEval > stack[ply - 2].staticEval);

  //=================== RFP ==================
  // so if we winning by a lot (eval-margin>beta) we assume that we win without
  // needing to search further
  if (!PvNode && !inCheck && depth <= 8 && pos.hasNonPawnMaterial<c>() &&
      abs(beta) < MATE_SCORE - 100) {
    // 1.3: Use tighter margin when not improving
    int margin = improving ? 120 * depth : 80 * depth;
    if (staticEval - margin >= beta) {
      return beta;
    }
  }

  //======================= NMP =============================
  // so if we pass and still win then we can beat easily
  if (!PvNode && depth >= 3 && !inCheck && ply > 0 && staticEval >= beta &&
      pos.hasNonPawnMaterial<c>()) {
    // 1.3: Reduce more when improving
    int R = 3 + (depth / 6) + improving;
    // if we win more then reduce more
    if (staticEval >= beta + 200)
      R++;

    // also we have ensure that depth do not drop to 0
    if (depth - R - 1 > 0) {
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
        return score;
      }
    }
  }

  //================= FP ==================
  // so if a position is bad (eval+margin<alpha) quiet moves can`t save us
  bool futilityprun = false;
  if (depth <= 4 && !inCheck && !PvNode && abs(alpha) < MATE_SCORE - 100 &&
      abs(beta) < MATE_SCORE - 100) {
    // so margin if we improve by 1.5 pawns per depth
    int margin = depth * 150;
    if (staticEval + margin <= alpha) {
      futilityprun = true;
    }
  }

  // Move generation
  MoveList moves;
  gen.generate_all_moves<c>(pos, moves);
  if (moves.empty())
    return pos.inCheck<c>() ? -MATE_SCORE + ply : 0;

  orderer.scoreMoves(pos, moves, ttMove, stack[ply].killers, history,
                     previousMove, counterMoves);

  int originalAlpha = alpha;
  Move bestMove = NO_MOVE;
  int bestScore = -INFINITE;
  int legalMoves = 0;

  // lmp counter for quiet move
  int quietmovesearched = 0;

  // 1.4: Track quiet moves searched for history malus
  Move searchedQuiets[64];
  int searchedQuietCount = 0;

  for (const Move move : moves) {

    //============== LMP ===============
    if (!PvNode && !inCheck && depth < 8 && !move.is_capture() &&
        !move.is_promotion()) {
      // 1.3: Be more aggressive when not improving
      int lmpthre = improving ? (3 + depth * depth) : (3 + depth * depth / 2);
      if (quietmovesearched >= lmpthre) {
        continue; // skip the move
      }
    }

    pos.makemove<c>(move);

    if (pos.inCheck<c>()) {
      pos.unmakemove<c>(move);
      continue;
    }

    legalMoves++;

    // now we have to increment quiet move
    if (!move.is_capture() && !move.is_promotion()) {
      quietmovesearched++;
      // 1.4: Track quiet moves for history malus
      if (searchedQuietCount < 64) {
        searchedQuiets[searchedQuietCount++] = move;
      }
    }

    bool givesCheck = pos.inCheck<~c>();
    if (futilityprun && legalMoves > 0 && !move.is_capture() &&
        !move.is_promotion() && !givesCheck) {
      pos.unmakemove<c>(move);
      continue;
    }

    int score;
    bool needfullsearch = true;

    // ============================ LMR =========================
    //  Use pre computed logarithmic LMR table
    if (depth >= 3 && legalMoves > 4 && !PvNode && !inCheck &&
        !move.is_capture() && !move.is_promotion() && !givesCheck &&
        extension == 0) {

      int R = lmrTable[std::min(depth, 63)][std::min(legalMoves, 63)];

      // LMR adjustments
      int historyScore = history[c][move.from()][move.to()];
      if (cutNode)
        R += 1;
      if (historyScore < 0)
        R += 1;
      if (historyScore > 4000)
        R -= 1;
      // Increase reduction when not improving
      if (!improving)
        R += 1;

      R = std::clamp(R, 0, depth - 2);

      // also don`t reduce when depth<1
      int reduceddepth = std::max(1, depth - 1 - R);

      // now search with LMR
      score = -pvs<~c, false>(reduceddepth, ply + 1, -alpha - 1, -alpha, true,
                              move);
      // so now check if score>alpha then our reduction is wrong and move is
      // good
      if (score > alpha) {
        needfullsearch = true;
      } else {
        needfullsearch = false;
      }
    }
    // now full depth
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
          if (!move.is_capture() && ply < MAX_PLY) {
            stack[ply].killers[1] = stack[ply].killers[0];
            stack[ply].killers[0] = move;

            // gravity based history update (naturally bounded)
            int bonus = std::min(depth * depth, 400);
            int side = (c == White) ? 0 : 1;

            // Bonus for the move that caused the cutoff
            history[side][move.from()][move.to()] +=
                bonus -
                history[side][move.from()][move.to()] * std::abs(bonus) / 16384;

            // Malus for all searched quiets that didn't cut
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

  // Store to TT
  int flag = (bestScore >= beta)           ? HASH_FLAG_BETA
             : (bestScore > originalAlpha) ? HASH_FLAG_EXACT
                                           : HASH_FLAG_ALPHA;
  tt.store(pos.hash(), depth, flag, bestScore, 0, ply, bestMove);

  return bestScore;
}

template <Color c> int Searcher::quiescence(int alpha, int beta, int ply) {
  // Hard limit to prevent infinite recursion
  if (ply >= MAX_PLY - 1) {
    return eval.evaluate_board(pos);
  }

  nodes++;

  // Check time periodically
  if ((nodes & 2047) == 0) {
    check_time();
  }
  if (stopFlag)
    return 0;

  bool inCheck = pos.inCheck<c>();

  // Stand pat (only if not in check)
  int standPat = -INFINITE;
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

  // Move generation: when in check generate all moves, otherwise only captures+promotions
  MoveList movelist;
  if (inCheck) {
    gen.generate_all_moves<c>(pos, movelist);
  } else {
    gen.generate_captures<c>(pos, movelist);
  }

  if (movelist.empty()) {
    return inCheck ? (-MATE_SCORE + ply) : alpha;
  }

  // Sort moves for ordering
  if (inCheck) {
    Move ttMove = NO_MOVE;
    orderer.scoreMoves(pos, movelist, ttMove, stack[ply].killers,
                       history, NO_MOVE, counterMoves);
  } else {
    orderer.scoreCaptures(pos, movelist);
  }

  // now iterate
  int legalMoves = 0;
  for (const Move &move : movelist) {
    // Make the move
    pos.makemove<c>(move);

    // check if move is legal (doesn't leave king in check)
    if (pos.inCheck<c>()) {
      pos.unmakemove<c>(move);
      continue;
    }

    legalMoves++;

    // Recursive quiescence search with negamax framework
    int score = -quiescence<~c>(-beta, -alpha, ply + 1);

    // Unmake the move
    pos.unmakemove<c>(move);

    // if(stopFlag) return 0;

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
  // checks if hardTime exceeded
  tm.Check();

  // stops when hard time reached
  if (tm.StopFlag()) {
    stopFlag = true;
  }
}

void Searcher::update_uci_info(int depth, int score, const PVLine &pv) {
  std::cout << "info depth " << depth;

  // Mate score detection
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
  clearHistory();
  for (int i = 0; i < MAX_PLY + 10; i++) {
    stack[i].clear();
  }
}

}; // namespace Search
