#pragma once

#include "psqt.h"       // Piece-square tables
#include "../board/position.h"
#include <cstdint>
#include <array>

namespace ASTROVE {
namespace eval{
    using EvalScore = int32_t;
    using Score = int16_t;

    // Compose a combined evaluation with opening and endgame parts
    constexpr EvalScore composeEval(Score opening, Score endgame) {
        return static_cast<EvalScore>((static_cast<int32_t>(endgame) << 16) | static_cast<uint16_t>(opening));
    }

    // Extract opening and endgame from combined EvalScore
    constexpr Score openingScore(EvalScore score) {
        return static_cast<Score>(static_cast<int16_t>(score & 0xFFFF));
    }
    constexpr Score endgameScore(EvalScore score) {
        return static_cast<Score>(static_cast<int16_t>(score >> 16));
    }

    // Evaluation constants
    constexpr EvalScore TEMPO_BONUS = composeEval(10, 10);

    // Base piece values (tapered)
    constexpr EvalScore PieceValues[6] = {
        composeEval(100, 100),   // Pawn
        composeEval(320, 320),   // Knight
        composeEval(330, 330),   // Bishop
        composeEval(500, 500),   // Rook
        composeEval(900, 900),   // Queen
        composeEval(0,   0)      // King
    };

    struct EvaluationData {
        EvalScore currentScore = composeEval(0, 0);

        void add(EvalScore s) {
            currentScore += s;
        }
        Score opening() const {
            return openingScore(currentScore);
        }
        Score endgame() const {
            return endgameScore(currentScore);
        }
    };

    class Evaluator {
    public:
        Evaluator() = default;
        ~Evaluator() = default;

        Score evaluate_board(const Position& pos);

    private:
        EvaluationData evalData;

        void initialize(const Position& pos);
        void evaluate_material_and_placement(const Position& pos);
        // Placeholder for future detailed evaluation functions
        void evaluate_pawns(const Position& pos);
        void evaluate_knights(const Position& pos);
        void evaluate_bishops(const Position& pos);
        void evaluate_rooks(const Position& pos);
        void evaluate_queens(const Position& pos);
        void evaluate_king_safety(const Position& pos);

        int calculate_game_phase(const Position& pos) const;
        Score calculate_final_score(const Position& pos) const;
    };

    // Global evaluator instance
    extern Evaluator board_evaluator;

    // Helper function
    inline Score evaluate(const Position& pos) {
        return board_evaluator.evaluate_board(pos);
    }
} //namespace eval
} //namespace ASTROVE
