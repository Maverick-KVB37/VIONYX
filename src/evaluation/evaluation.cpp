#include "evaluation.h"
#include "../core/types.h"
#include "psqt.h"
#include <algorithm>

namespace ASTROVE::eval {

    Evaluator board_evaluator;

    Score Evaluator::evaluate_board(const Position& pos) {
        initialize(pos);

        evaluate_material_and_placement(pos);

        // Uncomment and implement the following for richer evaluation
        // evaluate_pawns(pos);
        // evaluate_knights(pos);
        // evaluate_bishops(pos);
        // evaluate_rooks(pos);
        // evaluate_queens(pos);
        // evaluate_king_safety(pos);

        return calculate_final_score(pos);
    }

    void Evaluator::initialize(const Position& pos) {
        evalData = EvaluationData{};
    }

    void Evaluator::evaluate_material_and_placement(const Position& pos) {
        for (Color c = White; c <= Black; ++c) {
            for (PieceType pt = Pawn; pt <= Queen; ++pt) {
                Bitboard pieces = pos.pieces(c ,pt);
                while (pieces) {
                    Square sq = poplsb(pieces);
                    EvalScore materialVal = PieceValues[pt];
                    EvalScore psqtVal = PSQT[pt][c][sq];
                    evalData.add((c == White) ? (materialVal + psqtVal) : -(materialVal + psqtVal));
                }
            }
        }
    }

    int Evaluator::calculate_game_phase(const Position& pos) const {
        // Example piece phase values (can adjust based on tuning)
        constexpr int PiecePhaseValue[12] = {0, 1, 1, 2, 4, 0, 0, 1, 1, 2, 4, 0};
        int phase = 0;
        for (int i = 0; i < 12; ++i) {
            phase += popcount(pos.getPiecesBB(i)) * PiecePhaseValue[i];
        }
        return std::clamp(phase, 0, 24);
    }

    Score Evaluator::calculate_final_score(const Position& pos) const {
        EvalScore score = evalData.currentScore;

        // Add tempo bonus for side to move
        if (pos.sideToMove() == White) {
            score += TEMPO_BONUS;
        }

        int phase = calculate_game_phase(pos);
        constexpr int maxPhase = 24;

        Score opening = openingScore(score);
        Score endgame = endgameScore(score);

        Score finalScore = (opening * phase + endgame * (maxPhase - phase)) / maxPhase;

        return (pos.sideToMove() == White) ? finalScore : -finalScore;
    }

    // ----- Placeholders for future enhancements -----
    void Evaluator::evaluate_pawns(const Position& pos) {/* TODO */}
    void Evaluator::evaluate_knights(const Position& pos) {/* TODO */}
    void Evaluator::evaluate_bishops(const Position& pos) {/* TODO */}
    void Evaluator::evaluate_rooks(const Position& pos) {/* TODO */}
    void Evaluator::evaluate_queens(const Position& pos) {/* TODO */}
    void Evaluator::evaluate_king_safety(const Position& pos) {/* TODO */}

}
