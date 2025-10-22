#include "evaluation.h"
#include "../core/types.h"
#include "psqt.h"
#include <algorithm>

namespace ASTROVE::eval {

    Evaluator board_evaluator;

    Score Evaluator::evaluate_board(const Position& pos) {
        //std::cerr << "DEBUG: evaluate_board entered\n";
        initialize(pos);
        //std::cerr << "DEBUG: Initialized\n";
        evaluate_material_and_placement(pos);
        //std::cerr << "DEBUG: Material evaluated\n";

        // Uncomment and implement the following for richer evaluation
        // evaluate_pawns(pos);
        // evaluate_knights(pos);
        // evaluate_bishops(pos);
        // evaluate_rooks(pos);
        // evaluate_queens(pos);
        // evaluate_king_safety(pos);

        Score result = calculate_final_score(pos);
        //std::cerr << "DEBUG: Final score: " << result << "\n";
        
        return result;
    }

    void Evaluator::initialize(const Position& pos) {
        evalData = EvaluationData{};
    }
    /*
    void Evaluator::evaluate_material_and_placement(const Position& pos) {
        std::cerr << "DEBUG: evaluate_material_and_placement entered\n";
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
        std::cerr << "DEBUG: calculate_game_phase entered\n";
        constexpr int PiecePhaseValue[12] = {0, 1, 1, 2, 4, 0, 0, 1, 1, 2, 4, 0};
        int phase = 0;
        for (int i = 0; i < 12; ++i) {
            phase += popcount(pos.getPiecesBB(i)) * PiecePhaseValue[i];
        }
        return std::clamp(phase, 0, 24);
        std::cerr << "DEBUG: Game phase: " << phase << "\n";
    }

    Score Evaluator::calculate_final_score(const Position& pos) const {
        std::cerr << "DEBUG: calculate_final_score entered\n";
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
        std::cerr << "DEBUG: finalScore before perspective: " << finalScore << "\n";
        
        // Return from side to move perspective
        Score result = (pos.sideToMove() == White) ? finalScore : -finalScore;  // No parentheses!
        
        std::cerr << "DEBUG: calculate_final_score returning " << result << "\n";
        
        return result;
    }
    */
    
    void Evaluator::evaluate_material_and_placement(const Position& pos) {
    //std::cerr << "DEBUG: evaluate_material_and_placement entered\n";
    
    // Iterate through all squares instead
    for (Square sq = SQ_A1; sq <= SQ_H8; sq = Square(sq + 1)) {
        Piece piece = pos.pieceAt(sq);
        if (piece == None) continue;
        
        Color c = piececolor(piece);
        PieceType pt = piecetype(piece);
        
        if (pt == King) continue;  // Don't evaluate king material
        
        EvalScore materialVal = PieceValues[pt];
        EvalScore psqtVal = PSQT[pt][c][sq];
        
        evalData.add((c == White) ? (materialVal + psqtVal) : -(materialVal + psqtVal));
    }
    
    //std::cerr << "DEBUG: evaluate_material_and_placement done\n";
    }


int Evaluator::calculate_game_phase(const Position& pos) const {
    //std::cerr << "DEBUG: calculate_game_phase entered\n";
    
    int phase = 0;
    phase += popcount(pos.knights<White>()) + popcount(pos.knights<Black>());
    phase += popcount(pos.bishops<White>()) + popcount(pos.bishops<Black>());
    phase += 2 * (popcount(pos.rooks<White>()) + popcount(pos.rooks<Black>()));
    phase += 4 * (popcount(pos.queens<White>()) + popcount(pos.queens<Black>()));
    
    phase = std::clamp(phase, 0, 24);
    //std::cerr << "DEBUG: Game phase: " << phase << "\n";
    
    return phase;
}

Score Evaluator::calculate_final_score(const Position& pos) const {
    //std::cerr << "DEBUG: calculate_final_score entered\n";
    
    EvalScore score = evalData.currentScore;

    // Add tempo bonus for side to move (FIX: no parentheses!)
    if (pos.sideToMove() == White) {
        score += TEMPO_BONUS;
    }

    int phase = calculate_game_phase(pos);
    constexpr int maxPhase = 24;

    Score opening = openingScore(score);
    Score endgame = endgameScore(score);

    Score finalScore = (opening * phase + endgame * (maxPhase - phase)) / maxPhase;
    
    //std::cerr << "DEBUG: finalScore before perspective: " << finalScore << "\n";
    
    // Return from side to move perspective (FIX: no parentheses!)
    Score result = (pos.sideToMove() == White) ? finalScore : -finalScore;
    
    //std::cerr << "DEBUG: calculate_final_score returning " << result << "\n";
    
    return result;
}

    // ----- Placeholders for future enhancements -----
    void Evaluator::evaluate_pawns(const Position& pos) {/* TODO */}
    void Evaluator::evaluate_knights(const Position& pos) {/* TODO */}
    void Evaluator::evaluate_bishops(const Position& pos) {/* TODO */}
    void Evaluator::evaluate_rooks(const Position& pos) {/* TODO */}
    void Evaluator::evaluate_queens(const Position& pos) {/* TODO */}
    void Evaluator::evaluate_king_safety(const Position& pos) {/* TODO */}

}
