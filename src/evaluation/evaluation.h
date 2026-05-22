#pragma once

#include "psqt.h"
#include "../board/position.h"
#include <cstdint>
#include <array>

namespace ASTROVE {
namespace eval{

    //evaluation constants
    constexpr EvalScore TEMPO_BONUS = composeEval(20, 10);

    //penalty for pawn
    constexpr EvalScore ISOLATED_PAWN_PENALTY= composeEval(-15,-20);
    constexpr EvalScore DOUBLED_PAWN_PENALTY=composeEval(-10,-20);
    constexpr EvalScore BACKWARD_PAWN_PENALTY = composeEval(-10,-20);

    //king safety
    constexpr EvalScore KING_PAWN_SHIELD_PENALTY=composeEval(10,0);
    constexpr EvalScore KING_OPEN_FILE_PENALTY=composeEval(25,0);

    //rook on open file
    constexpr EvalScore ROOK_OPEN_FILE_BONUS=composeEval(20,40);
    constexpr EvalScore ROOK_SEMI_OPEN_FILE_BONUS=composeEval(10,20);

    //outpost const
    constexpr EvalScore BISHOP_PAIR_BONUS=composeEval(30,40);

    constexpr int KNIGHT_OUTPOST_BONUS[8]={
        composeEval(0,0),
        composeEval(0,0),
        composeEval(10,5),
        composeEval(30,15),
        composeEval(40,20),
        composeEval(30,15),
        composeEval(10,5),
        composeEval(0,0)
    };

    //base piece values (tapered)
    constexpr EvalScore PieceValues[6] = {
        composeEval(100, 100),   // Pawn
        composeEval(320, 280),   // Knight
        composeEval(330, 300),   // Bishop
        composeEval(500, 520),   // Rook
        composeEval(900, 900),   // Queen
        composeEval(0,   0)      // King
    };

    //bonuses (Passed pawns are deadly in endgame)
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

    //MOBILITY TABLE
    //for knight 0-8 moves
    constexpr EvalScore MobilityBonus_Knight[9]={
        composeEval(-20,-30),
        composeEval(-10,-10),
        composeEval(0,0),
        composeEval(5,5),
        composeEval(10,10),
        composeEval(15,15),
        composeEval(20,20),
        composeEval(25,25),
        composeEval(30,30)
    };

    //for bishop 0-13 moves
    constexpr EvalScore MobilityBonus_Bishop[14]={
        composeEval(-20,-30),
        composeEval(-10,-15),
        composeEval(0,-5),
        composeEval(5,0),
        composeEval(10,5),
        composeEval(15,10),
        composeEval(20,15),
        composeEval(25,20),
        composeEval(30,25),
        composeEval(35,30),
        composeEval(40,35),
        composeEval(45,40),
        composeEval(50,45),
        composeEval(50,50)
    };

    //for rook 0-14 moves
    constexpr EvalScore MobilityBonus_Rook[15]={
        composeEval(-10,-20),
        composeEval(-5,-10),
        composeEval(0,0),
        composeEval(5,5),
        composeEval(10,10),
        composeEval(15,15),
        composeEval(20,20),
        composeEval(25,25),
        composeEval(30,30),
        composeEval(35,35),
        composeEval(40,40),
        composeEval(45,45),
        composeEval(50,50),
        composeEval(55,55),
        composeEval(60,60)
    };

    //attacks weights for king attacks
    constexpr int knightweight=2;
    constexpr int bishopweight=2;
    constexpr int rookweight=3;
    constexpr int queenweight=5;

    //king safety table(indec=attacks units(weighted sum of attackers))
    //value penalty score{middle game ,endgame}
    constexpr EvalScore SafetyTable[100] = {
        composeEval(0, 0),  composeEval(0, 0),  composeEval(0, 0),  composeEval(1, 0),
        composeEval(2, 0),  composeEval(3, 0),  composeEval(5, 0),  composeEval(7, 0),
        composeEval(9, 0),  composeEval(12, 0), composeEval(15, 0), composeEval(18, 0),
        composeEval(22, 0), composeEval(26, 0), composeEval(30, 0), composeEval(35, 0),
        composeEval(40, 0), composeEval(45, 0), composeEval(50, 0), composeEval(55, 0),
        composeEval(60, 0), composeEval(65, 0), composeEval(70, 0), composeEval(75, 0),
        composeEval(80, 0), composeEval(85, 0), composeEval(90, 0), composeEval(95, 0),
        composeEval(100, 0),composeEval(110, 0),composeEval(120, 0),composeEval(130, 0),
        composeEval(140, 0),composeEval(150, 0),composeEval(160, 0),composeEval(170, 0),
        composeEval(180, 0),composeEval(190, 0),composeEval(200, 0),composeEval(200, 0),
        composeEval(200, 0),composeEval(200, 0),composeEval(200, 0),composeEval(200, 0),
        composeEval(200, 0),composeEval(200, 0),composeEval(200, 0),composeEval(200, 0),
        composeEval(200, 0),composeEval(200, 0),composeEval(200, 0),composeEval(200, 0),
        composeEval(200, 0),composeEval(200, 0),composeEval(200, 0),composeEval(200, 0),
        composeEval(200, 0),composeEval(200, 0),composeEval(200, 0),composeEval(200, 0),
        composeEval(200, 0),composeEval(200, 0),composeEval(200, 0),composeEval(200, 0),
        composeEval(200, 0),composeEval(200, 0),composeEval(200, 0),composeEval(200, 0),
        composeEval(200, 0),composeEval(200, 0),composeEval(200, 0),composeEval(200, 0),
        composeEval(200, 0),composeEval(200, 0),composeEval(200, 0),composeEval(200, 0),
        composeEval(200, 0),composeEval(200, 0),composeEval(200, 0),composeEval(200, 0),
        composeEval(200, 0),composeEval(200, 0),composeEval(200, 0),composeEval(200, 0),
        composeEval(200, 0),composeEval(200, 0),composeEval(200, 0),composeEval(200, 0),
        composeEval(200, 0),composeEval(200, 0),composeEval(200, 0),composeEval(200, 0),
        composeEval(200, 0),composeEval(200, 0),composeEval(200, 0),composeEval(200, 0),
        composeEval(200, 0),composeEval(200, 0),composeEval(200, 0),composeEval(200, 0)
    };


    struct EvaluationData {
        int32_t mg=0;
        int32_t eg=0;

        void add(EvalScore s) {
            mg+=static_cast<int16_t>(s.value& 0xFFFF);
            eg+=static_cast<int16_t>((s.value >> 16)& 0xFFFF);
        }

        void add(int s){
            mg+=s;
            eg+=s;
        }

        void subtract(EvalScore s) {
            mg-=static_cast<int16_t>(s.value& 0xFFFF);
            eg-=static_cast<int16_t>((s.value>>16)& 0xFFFF);
        }

        void subtract(int s){
            mg-=s;
            eg-=s;
        }
        
        Score opening() const {
            return static_cast<Score>(mg);
        }

        Score endgame() const {
            return static_cast<Score>(eg);
        }
    };

    class Evaluator {
    public:
        Evaluator() = default;
        ~Evaluator() = default;
        Score EvaluateBoard(const Position& pos);

    private:
        EvaluationData evalData;

        void initialize(const Position& pos);

        void EvaluateMaterialAndPlacement(const Position& pos);
        void EvaluatePawns(const Position& pos);
        void EvaluateMobility(const Position& pos);
        void EvaluateKingSafety(const Position& pos);
        void EvaluateRook(const Position& pos);
        void EvaluatePieceStructure(const Position& pos);

        int CalculateGamePhase(const Position& pos) const;
        Score CalculateFinalScore(const Position& pos) const;
    };

    //global instance
    extern Evaluator board_evaluator;

    inline Score evaluate(const Position& pos) {
        return board_evaluator.EvaluateBoard(pos);
    }
} //namespace eval
} //namespace ASTROVE
