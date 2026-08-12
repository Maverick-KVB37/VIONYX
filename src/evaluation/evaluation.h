#pragma once

#include "../board/position.h"
#include "../table/pawnhash.h"
#include "psqt.h"
#include <array>
#include <cstdint>

namespace ASTROVE {
namespace eval {

// evaluation constants
constexpr EvalScore TEMPO_BONUS = composeEval(20, 10);

// penalty for pawn
constexpr EvalScore ISOLATED_PAWN_PENALTY = composeEval(-15, -20);
constexpr EvalScore DOUBLED_PAWN_PENALTY = composeEval(-5, -10);
constexpr EvalScore BACKWARD_PAWN_PENALTY = composeEval(-10, -20);

// king safety
constexpr EvalScore KING_PAWN_SHIELD_PENALTY = composeEval(10, 0);
constexpr EvalScore KING_OPEN_FILE_PENALTY = composeEval(25, 0);

// rook on open file
constexpr EvalScore ROOK_OPEN_FILE_BONUS = composeEval(20, 40);
constexpr EvalScore ROOK_SEMI_OPEN_FILE_BONUS = composeEval(10, 20);

// outpost const
constexpr EvalScore BISHOP_PAIR_BONUS = composeEval(30, 40);

// attacks weights for king attacks
constexpr int knightweight = 2;
constexpr int bishopweight = 2;
constexpr int rookweight = 3;
constexpr int queenweight = 5;


constexpr EvalScore KNIGHT_OUTPOST_BONUS[8] = {
    composeEval(0, 0),   composeEval(0, 0),   composeEval(10, 5),
    composeEval(30, 15), composeEval(40, 20), composeEval(30, 15),
    composeEval(10, 5),  composeEval(0, 0)};

// base piece values (tapered)
constexpr EvalScore PieceValues[6] = {
    composeEval(100, 100), // Pawn
    composeEval(320, 280), // Knight
    composeEval(330, 300), // Bishop
    composeEval(500, 520), // Rook
    composeEval(900, 900), // Queen
    composeEval(0, 0)      // King
};

// bonuses (Passed pawns are deadly in endgame)
constexpr EvalScore PASSED_PAWN_BONUS[8] = {
    composeEval(0, 0),     // Rank 1 (impossible)
    composeEval(5, 10),    // Rank 2
    composeEval(10, 20),   // Rank 3
    composeEval(20, 40),   // Rank 4
    composeEval(40, 70),   // Rank 5
    composeEval(80, 140),  // Rank 6
    composeEval(150, 240), // Rank 7
    composeEval(0, 0)      // Rank 8 (promoted)
};

// MOBILITY TABLE
// for knight 0-8 moves
constexpr EvalScore MobilityBonus_Knight[9] = {
    composeEval(-20, -30), composeEval(-10, -10), composeEval(0, 0),
    composeEval(5, 5),     composeEval(10, 10),   composeEval(15, 15),
    composeEval(20, 20),   composeEval(25, 25),   composeEval(30, 30)
};

// for bishop 0-13 moves
constexpr EvalScore MobilityBonus_Bishop[14] = {
    composeEval(-20, -30), composeEval(-10, -15), composeEval(0, -5),
    composeEval(5, 0),     composeEval(10, 5),    composeEval(15, 10),
    composeEval(20, 15),   composeEval(25, 20),   composeEval(30, 25),
    composeEval(35, 30),   composeEval(40, 35),   composeEval(45, 40),
    composeEval(50, 45),   composeEval(50, 50)
};

// for rook 0-14 moves
constexpr EvalScore MobilityBonus_Rook[15] = {
    composeEval(-10, -20), composeEval(-5, -10), composeEval(0, 0),
    composeEval(5, 5),     composeEval(10, 10),  composeEval(15, 15),
    composeEval(20, 20),   composeEval(25, 25),  composeEval(30, 30),
    composeEval(35, 35),   composeEval(40, 40),  composeEval(45, 45),
    composeEval(50, 50),   composeEval(55, 55),  composeEval(60, 60)
};

// King safety table (index = attack units)
// Value: penalty score {middle game, endgame}
constexpr EvalScore SafetyTable[100] = {
    composeEval(0, 0),     composeEval(0, 0),     composeEval(1, 0),     composeEval(2, 0),     composeEval(3, 0),
    composeEval(5, 0),     composeEval(7, 0),     composeEval(9, 0),     composeEval(12, 1),    composeEval(15, 1),
    composeEval(18, 1),    composeEval(22, 2),    composeEval(26, 2),    composeEval(30, 3),    composeEval(35, 3),
    composeEval(39, 3),    composeEval(44, 4),    composeEval(50, 5),    composeEval(56, 5),    composeEval(62, 6),
    composeEval(68, 6),    composeEval(75, 7),    composeEval(82, 8),    composeEval(85, 8),    composeEval(89, 8),
    composeEval(97, 9),    composeEval(105, 10),  composeEval(113, 11),  composeEval(122, 12),  composeEval(131, 13),
    composeEval(140, 14),  composeEval(150, 15),  composeEval(169, 16),  composeEval(180, 18),  composeEval(191, 19),
    composeEval(202, 20),  composeEval(213, 21),  composeEval(225, 22),  composeEval(237, 23),  composeEval(248, 24),
    composeEval(260, 26),  composeEval(272, 27),  composeEval(283, 28),  composeEval(295, 29),  composeEval(307, 30),
    composeEval(319, 31),  composeEval(330, 33),  composeEval(342, 34),  composeEval(354, 35),  composeEval(366, 36),
    composeEval(377, 37),  composeEval(389, 38),  composeEval(401, 40),  composeEval(412, 41),  composeEval(424, 42),
    composeEval(436, 43),  composeEval(448, 44),  composeEval(459, 45),  composeEval(471, 47),  composeEval(483, 48),
    composeEval(494, 49),  composeEval(500, 50),  composeEval(500, 50),  composeEval(500, 50),  composeEval(500, 50),
    composeEval(500, 50),  composeEval(500, 50),  composeEval(500, 50),  composeEval(500, 50),  composeEval(500, 50),
    composeEval(500, 50),  composeEval(500, 50),  composeEval(500, 50),  composeEval(500, 50),  composeEval(500, 50),
    composeEval(500, 50),  composeEval(500, 50),  composeEval(500, 50),  composeEval(500, 50),  composeEval(500, 50),
    composeEval(500, 50),  composeEval(500, 50),  composeEval(500, 50),  composeEval(500, 50),  composeEval(500, 50),
    composeEval(500, 50),  composeEval(500, 50),  composeEval(500, 50),  composeEval(500, 50),  composeEval(500, 50),
    composeEval(500, 50),  composeEval(500, 50),  composeEval(500, 50),  composeEval(500, 50),  composeEval(500, 50),
    composeEval(500, 50),  composeEval(500, 50),  composeEval(500, 50),  composeEval(500, 50),  composeEval(500, 50)
};

// Queen mobility table (index = safe squares attacked 0 to 27)
// Value penalty/bonus score {middle game, endgame}
constexpr EvalScore MobilityBonus_Queen[28] = {
    composeEval(-30, -30), composeEval(-25, -25), composeEval(-20, -20), composeEval(-15, -15),
    composeEval(-10, -10), composeEval(-5, -5),   composeEval(0, 0),     composeEval(2, 2),
    composeEval(4, 4),     composeEval(6, 6),     composeEval(8, 8),     composeEval(10, 10),
    composeEval(12, 12),   composeEval(14, 14),   composeEval(16, 16),   composeEval(18, 18),
    composeEval(20, 20),   composeEval(22, 22),   composeEval(24, 24),   composeEval(25, 25),
    composeEval(26, 26),   composeEval(27, 27),   composeEval(28, 28),   composeEval(29, 29),
    composeEval(30, 30),   composeEval(31, 31),   composeEval(32, 32),   composeEval(33, 33)
};

class EvaluationData {
public:
  int32_t mg = 0;
  int32_t eg = 0;

  void add(EvalScore s) { //unpacks mg/eg correctly
    mg += static_cast<int16_t>(s.value & 0xFFFF);
    eg += static_cast<int16_t>((s.value >> 16) & 0xFFFF);
  }

  void add(int s) {// adds a single score to both mg and eg
    mg += s;
    eg += s;
  }

  void subtract(EvalScore s) {
    mg -= static_cast<int16_t>(s.value & 0xFFFF);
    eg -= static_cast<int16_t>((s.value >> 16) & 0xFFFF);
  }

  void subtract(int s) {
    mg -= s;
    eg -= s;
  }

  Score opening() const { return static_cast<Score>(mg); }

  Score endgame() const { return static_cast<Score>(eg); }
};

class Evaluator {
public:
  Evaluator() = default;
  ~Evaluator() = default;
  Score EvaluateBoard(const Position &pos);

  void clearPawnHash() { pawnTable.clear(); }

private:
  EvaluationData evalData;
  PawnHashTable pawnTable;

  void initialize(const Position &pos);

  void EvaluateMaterialAndPlacement(const Position &pos);
  void EvaluatePawns(const Position &pos);
  void EvaluateMobility(const Position &pos);
  void EvaluateKingSafety(const Position &pos);
  void EvaluateRook(const Position &pos);
  void EvaluatePieceStructure(const Position &pos);

  int CalculateGamePhase(const Position &pos) const;
  Score CalculateFinalScore(const Position &pos) const;
};

// global instance
extern Evaluator board_evaluator;

inline Score evaluate(const Position &pos) {
  return board_evaluator.EvaluateBoard(pos);
}
} // namespace eval
} // namespace ASTROVE
