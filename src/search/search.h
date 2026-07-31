#pragma once

#include "../board/movegen.h"
#include "../board/position.h"
#include "../evaluation/evaluation.h"
#include "../ordering/ordering.h"
#include "../table/tt.h"
#include "timemanager.h"

#include <algorithm>
#include <atomic>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <vector>

using ASTROVE::eval::Evaluator;

namespace Search {

constexpr int MAX_PLY = 256;
constexpr int INFINITE = 50000;
constexpr int MATE_SCORE = 49000;
constexpr int TB_WIN_SCORE = 48000;
constexpr int MATE_BOUND = MATE_SCORE - MAX_PLY;

class SearchLimits;
class PVLine;
class SearchStack;
class SearchInfo;

// Stores best play
class PVLine {
public:
  // 4-byte types
  int length = 0;

  // 2-byte element array
  Move moves[MAX_PLY];

  void clear() { length = 0; }

  void update(Move move, const PVLine &childPV) {
    moves[0] = move;
    length = 1;

    int maxCopy = std::min(childPV.length, MAX_PLY - 1);
    for (int i = 0; i < maxCopy; ++i) {
      moves[length] = childPV.moves[i];
      length++;
    }
  }
};

// Store data for a ply
class SearchStack {
public:
  // PVLine alignment is 4 bytes, so it pairs perfectly with ints
  PVLine pv;

  // 4-byte types
  int staticEval = 0;
  int moveCount = 0;
  int doubleExtensions = 0;

  // 2-byte types
  Move currentMove = NO_MOVE;
  Move excludedMove = NO_MOVE;
  Move killers[2] = {NO_MOVE, NO_MOVE};

  // 1-byte types
  bool inCheck = false;

  SearchStack() { clear(); }

  void clear() {
    currentMove = excludedMove = NO_MOVE;
    killers[0] = NO_MOVE;
    killers[1] = NO_MOVE;
    staticEval = moveCount = doubleExtensions = 0;
    inCheck = false;
    pv.clear();
  }
};

// Tells the limits for search
class SearchLimits {
public:
  // 8-byte types
  uint64_t nodes = UINT64_MAX;
  int64_t movetime = 0;
  int64_t movestogo = 0;
  int64_t wtime = 0;
  int64_t btime = 0;
  int64_t winc = 0;
  int64_t binc = 0;

  // 4-byte types
  int depth = MAX_PLY;
  int searchmovesCount = 0; // Replaces std::vector size()

  // 2-byte types
  Move searchmoves[256]; // Fixed array replaces dynamic std::vector

  // 1-byte types
  bool infinite = false;
  bool ponder = false;
};

// Collect data for uci output
class SearchInfo {
public:
  // 8-byte types MUST be at the top to prevent padding before PVLine
  uint64_t nodes = 0;
  uint64_t tbhits = 0;
  uint64_t nps = 0;

  // Composite types (4-byte alignment)
  PVLine pv;

  // 4-byte types
  int depth = 0;
  int seldepth = 0;
  int score = 0;
  int time = 0;
  int mateIn = 0;
  int hashfull = 0;

  // 1-byte types
  bool isMate = false;

  void clear() {
    depth = seldepth = score = time = mateIn = hashfull = 0;
    nodes = tbhits = nps = 0;
    isMate = false;
    pv.clear();
  }
};

class Searcher {
public:
  explicit Searcher(Position &pos, TranspositionTable &tt);

  Move think(const SearchLimits &limits);

  void stop() { stopFlag = true; }
  void newGame();

  void clearHistory() {
    for (int c = 0; c < 2; ++c) {
      for (int f = 0; f < 64; ++f) {
        for (int t = 0; t < 64; t++) {
          history[c][f][t] = 0;
          counterMoves[c][f][t] = NO_MOVE;
        }
      }
    }
  }

private:
  template <Color Us, bool PvNode>
  int pvs(int depth, int ply, int alpha, int beta, bool cutNode,
          Move previousMove);

  template <Color Us> int quiescence(int alpha, int beta, int ply);

  void IterativeDeepening();
  void initLmrTable();
  void ageHistory();
  void CheckTime();
  void UpdateUciInfo(int depth, int score, const PVLine &pv);
  bool IsDraw(int ply) const;

  int history[2][64][64]; //[color][from][to]
  int lmrTable[64][64];
  Move counterMoves[2][64][64];

  Position &pos;
  TranspositionTable &tt;

  Evaluator eval;
  MoveGenerator gen;
  TimeManager tm;
  MoveOrderer orderer;

  std::atomic<bool> stopFlag;
  SearchLimits limits;
  SearchInfo info;
  std::chrono::steady_clock::time_point startTime;
  uint64_t nodes;
  int selDepth;

  SearchStack stack[MAX_PLY + 10];
};

} // namespace Search
