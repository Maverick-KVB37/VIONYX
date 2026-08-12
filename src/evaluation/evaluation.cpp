#include "evaluation.h"
#include "../core/attacks.h"
#include "../core/types.h"
#include "psqt.h"
#include <algorithm>

namespace ASTROVE::eval {

Evaluator board_evaluator;

Score Evaluator::EvaluateBoard(const Position &pos) {
  initialize(pos);

  EvaluateMaterialAndPlacement(pos);
  EvaluatePawns(pos);
  EvaluateMobility(pos);
  EvaluateKingSafety(pos);
  EvaluateRook(pos);
  EvaluatePieceStructure(pos);

  return CalculateFinalScore(pos);
}

void Evaluator::initialize(const Position & /*pos*/) {
  evalData = EvaluationData{};
}

void Evaluator::EvaluateMaterialAndPlacement(const Position &pos) {
  evalData.mg += pos.getMgScore();
  evalData.eg += pos.getEgScore();
}

/*
| Evaluate Pawn Structure                                                     |
| Evaluates the pawn structure, storing and probing from the pawn hash table  |
| for efficiency. Considers isolated, doubled, passed, and backward pawns.    |
*/
void Evaluator::EvaluatePawns(const Position &pos) {
  // Probe pawn hash table
  int32_t cachedMg, cachedEg;
  if (pawnTable.probe(pos.getPawnKey(), cachedMg, cachedEg)) {
    evalData.mg += cachedMg;
    evalData.eg += cachedEg;
    return;
  }

  // Save current eval to compute delta
  int32_t mgBefore = evalData.mg;
  int32_t egBefore = evalData.eg;

  Bitboard whitePawns = pos.pawns<White>();
  Bitboard blackPawns = pos.pawns<Black>();

  // Precomputed adjacent file masks
  static const Bitboard ADJACENT_FILES[8] = {MASKFILE[1],
                                             MASKFILE[0] | MASKFILE[2],
                                             MASKFILE[1] | MASKFILE[3],
                                             MASKFILE[2] | MASKFILE[4],
                                             MASKFILE[3] | MASKFILE[5],
                                             MASKFILE[4] | MASKFILE[6],
                                             MASKFILE[5] | MASKFILE[7],
                                             MASKFILE[6]};

  // Evaluate White pawns
  Bitboard wp = whitePawns;
  while (wp) {
    Square sq = poplsb(wp);
    int f = fileof(sq);
    int r = rankof(sq);

    // Isolated pawn
    if ((whitePawns & ADJACENT_FILES[f]) == 0) {
      evalData.add(ISOLATED_PAWN_PENALTY);
    }

    // Doubled pawn
    if ((whitePawns & MASKFILE[f]) ^ (1ULL << sq)) {
      evalData.add(DOUBLED_PAWN_PENALTY);
    }

    // Passed pawn
    bool isPassed = (MASKPASSED[White][sq] & blackPawns) == 0;
    if(isPassed){
      evalData.add(PASSED_PAWN_BONUS[r]);
    }

    /*
    | Backward Pawn                                                            | 
    | Not a passed pawn (cannot safely advance)                                | 
    | No friendly pawns on adjacent files behind it (cannot be defended)       | 
    | Square in front is attacked by an enemy pawn (long-term weakness)        |
    */

    if(!isPassed){
      Bitboard ranksBehind = (1ULL << sq) - 1; // all squares with index < sq
      Bitboard friendsBehind = whitePawns & ADJACENT_FILES[f] & ranksBehind;

      // Square directly in front of the pawn
      Square frontSq = Square(sq + 8);
      Bitboard frontAttackByEnemy =
          Attacks::GetPawnAttacks(White, frontSq) & blackPawns;

      if (friendsBehind == 0 && frontAttackByEnemy != 0) {
        evalData.add(BACKWARD_PAWN_PENALTY);
      }
    }
  }

  // Evaluate Black pawns
  Bitboard bp = blackPawns;
  while (bp) {
    Square sq = poplsb(bp);
    int f = fileof(sq);
    int relative_rank = 7 - rankof(sq);

    // Isolated pawn
    if ((blackPawns & ADJACENT_FILES[f]) == 0) {
      evalData.subtract(ISOLATED_PAWN_PENALTY);
    }

    // Doubled pawn
    if ((blackPawns & MASKFILE[f]) ^ (1ULL << sq)) {
      evalData.subtract(DOUBLED_PAWN_PENALTY);
    }

    // Passed pawn
    bool isPassed = (MASKPASSED[Black][sq] & whitePawns) == 0;
    if(isPassed){
      evalData.subtract(PASSED_PAWN_BONUS[relative_rank]);
    }

    if (!isPassed) {
      Bitboard ranksAhead =
          ~((1ULL << (sq + 1)) - 1); // all squares with index > sq
      Bitboard friendsBehind = blackPawns & ADJACENT_FILES[f] & ranksAhead;

      // Square directly in front of the pawn
      Square frontSq = Square(sq - 8);
      Bitboard frontAttackByEnemy =
          Attacks::GetPawnAttacks(Black, frontSq) & whitePawns;

      if (friendsBehind == 0 && frontAttackByEnemy != 0) {
        evalData.subtract(BACKWARD_PAWN_PENALTY);
      }
    }
  }

  // Store pawn eval delta in pawn hash table
  pawnTable.store(pos.getPawnKey(), evalData.mg - mgBefore,
                  evalData.eg - egBefore);
}

void Evaluator::EvaluateMobility(const Position &pos) {
  Bitboard occupancy = pos.occupancy();

  //Pawn Mobility IS EVALUATED IN PAWN STRUCTURE EVALUATION, SO ONLY NEED TO EVALUATE KNIGHTS, BISHOPS, AND ROOKS
  Bitboard blackPawns =   pos.pawns<Black>();
  Bitboard blackPawnAtt = ((blackPawns & ~MASKFILE[FILE_H]) >> 7) | ((blackPawns & ~MASKFILE[FILE_A]) >> 9);

  Bitboard whitePawns =   pos.pawns<White>();
  Bitboard whitePawnAtt = ((whitePawns & ~MASKFILE[FILE_A]) << 7) | ((whitePawns & ~MASKFILE[FILE_H]) << 9);

  Bitboard whiteSafe = ~pos.occupancy(White) & ~blackPawnAtt;
  Bitboard blackSafe = ~pos.occupancy(Black) & ~whitePawnAtt;

  // WHITE KNIGHT MOBILITY
  Bitboard whiteknights = pos.knights<White>();
  while (whiteknights) {
    Square sq = poplsb(whiteknights);

    int count = popcount(Attacks::GetKnightAttacks(sq) & whiteSafe);
    evalData.add(MobilityBonus_Knight[count]);
  }

  //WHITE BISHOPS MOBILITY
  Bitboard whitebishops = pos.bishops<White>();
  while (whitebishops) {
    Square sq = poplsb(whitebishops);

    int count = popcount(Attacks::GetBishopAttacks(sq, occupancy) & whiteSafe);
    evalData.add(MobilityBonus_Bishop[std::min(count, 13)]);
  }

  //WHITE ROOKS MOBILITY
  Bitboard whiterooks = pos.rooks<White>();
  while (whiterooks) {
    Square sq = poplsb(whiterooks);

    int count = popcount(Attacks::GetRookAttacks(sq, occupancy) & whiteSafe);
    evalData.add(MobilityBonus_Rook[std::min(count, 14)]);
  }

  //WHITE QUEEN MOBILITY
  Bitboard whiteQueens = pos.queens<White>();
  while (whiteQueens) {
    Square sq = poplsb(whiteQueens);
    
    // A queen's attacks are the bitwise OR of rook and bishop attacks
    Bitboard attacks = Attacks::GetBishopAttacks(sq, occupancy) | Attacks::GetRookAttacks(sq, occupancy);
    int count = popcount(attacks & whiteSafe);
    
    // Max queen mobility is 27 squares
    evalData.add(MobilityBonus_Queen[std::min(count, 27)]);
  }

  // BLACK KNIGHT MOBILITY
  Bitboard blackknights = pos.knights<Black>();
  while (blackknights) {
    Square sq = poplsb(blackknights);

    int count = popcount(Attacks::GetKnightAttacks(sq) & blackSafe);
    evalData.subtract(MobilityBonus_Knight[count]);
  }

  // BLACK BISHOPS MOBILITY
  Bitboard blackbishops = pos.bishops<Black>();
  while (blackbishops) {
    Square sq = poplsb(blackbishops);

    int count = popcount(Attacks::GetBishopAttacks(sq, occupancy) & blackSafe);
    evalData.subtract(MobilityBonus_Bishop[std::min(count, 13)]);
  }

  // BLACK ROOKS MOBILITY
  Bitboard blackrooks = pos.rooks<Black>();
  while (blackrooks) {
    Square sq = poplsb(blackrooks);

    int count = popcount(Attacks::GetRookAttacks(sq, occupancy) & blackSafe);
    evalData.subtract(MobilityBonus_Rook[std::min(count, 14)]);
  }

  // BLACK QUEEN MOBILITY
  Bitboard blackQueens = pos.queens<Black>();
  while (blackQueens) {
    Square sq = poplsb(blackQueens);
    
    Bitboard attacks = Attacks::GetBishopAttacks(sq, occupancy) | Attacks::GetRookAttacks(sq, occupancy);
    int count = popcount(attacks & blackSafe);
    evalData.subtract(MobilityBonus_Queen[std::min(count, 27)]);
  }
}

/*
| Evaluate King Safety                                                        |
| Evaluates king safety by checking the king ring and calculating danger      |
| scores based on the number and type of enemy attackers near the king.       |
*/
void Evaluator::EvaluateKingSafety(const Position &pos) {
  Bitboard occupancy = pos.occupancy();

  // Helper lambda to calculate danger score for a specific side
  auto calculatedanger = [&](Color side) -> EvalScore {
    Square ksq = (side == White) ? pos.kingsq<White>() : pos.kingsq<Black>();
    Color enemy = ~side;

    // King ring
    Bitboard kingring = Attacks::GetKingAttacks(ksq);

    // Count attackers
    int attacksunit = 0;
    int attackercount = 0;

    // Knight attacks
    Bitboard knights = pos.pieces(enemy, Knight);
    while (knights) {
      Square sq = poplsb(knights);
      Bitboard attacks = Attacks::GetKnightAttacks(sq);

      if (attacks & kingring) {
        attacksunit += knightweight;
        attackercount++;
      }
    }

    // Bishop attacks
    Bitboard bishops = pos.pieces(enemy, Bishop);
    while (bishops) {
      Square sq = poplsb(bishops);
      Bitboard attacks = Attacks::GetBishopAttacks(sq, occupancy);

      if (attacks & kingring) {
        attacksunit += bishopweight;
        attackercount++;
      }
    }

    // Rook attacks
    Bitboard rooks = pos.pieces(enemy, Rook);
    while (rooks) {
      Square sq = poplsb(rooks);
      Bitboard attacks = Attacks::GetRookAttacks(sq, occupancy);

      if (attacks & kingring) {
        attacksunit += rookweight;
        attackercount++;
      }
    }

    // Queen attacks
    Bitboard queens = pos.pieces(enemy, Queen);
    while (queens) {
      Square sq = poplsb(queens);

      Bitboard attacks = Attacks::GetBishopAttacks(sq, occupancy) |
                         Attacks::GetRookAttacks(sq, occupancy);

      if (attacks & kingring) {
        attacksunit += queenweight;
        attackercount++;
      }
    }

    // Calculate penalty
    if (attackercount >= 1) {
      if (pos.pieces(enemy, Queen) == 0) {
        attacksunit /= 2;
      }

      int index = std::min(attacksunit + (attackercount * 3), 99);
      return SafetyTable[index];
    }
    return composeEval(0, 0);
  };

  // Apply penalties
  EvalScore whitedanger = calculatedanger(White);
  evalData.subtract(whitedanger);

  EvalScore blackdanger = calculatedanger(Black);
  evalData.add(blackdanger);
}

void Evaluator::EvaluateRook(const Position &pos) {
  // Evaluate White rooks
  Bitboard wrook = pos.rooks<White>();
  while (wrook) {
    Square sq = poplsb(wrook);
    int f = fileof(sq);
    Bitboard fileMask = MASKFILE[f];

    if (!(pos.pawns<White>() & fileMask)) {
      if (!(pos.pawns<Black>() & fileMask)) {
        evalData.add(ROOK_OPEN_FILE_BONUS);
      } else {
        evalData.add(ROOK_SEMI_OPEN_FILE_BONUS);
      }
    }
  }

  // Evaluate Black rooks
  Bitboard brook = pos.rooks<Black>();
  while (brook) {
    Square sq = poplsb(brook);
    int f = fileof(sq);
    Bitboard fileMask = MASKFILE[f];

    if (!(pos.pawns<Black>() & fileMask)) {
      if (!(pos.pawns<White>() & fileMask)) {
        evalData.subtract(ROOK_OPEN_FILE_BONUS);
      } else {
        evalData.subtract(ROOK_SEMI_OPEN_FILE_BONUS);
      }
    }
  }
}

/*
| Evaluate Piece Structure                                                    |
| Evaluates specific piece structures such as the bishop pair bonus and       |
| knight outposts supported by friendly pawns.                                |
*/
void Evaluator::EvaluatePieceStructure(const Position &pos) {
  // Bishop pair bonus
  if (popcount(pos.bishops<White>()) >= 2) {
    evalData.add(BISHOP_PAIR_BONUS);
  }

  if (popcount(pos.bishops<Black>()) >= 2) {
    evalData.subtract(BISHOP_PAIR_BONUS);
  }

  // Knight outpost bonus for White
  Bitboard wknight = pos.knights<White>();
  Bitboard wpawns = pos.pawns<White>();
  while (wknight) {
    Square sq = poplsb(wknight);
    int r = rankof(sq);

    // Check outposts on ranks 3 to 6
    if (r >= RANK_3 && r <= RANK_6) {
      if (Attacks::GetPawnAttacks(Black, sq) & wpawns) {
        // Must be supported by a pawn
        evalData.add(KNIGHT_OUTPOST_BONUS[r]);
      }
    }
  }

  Bitboard bknight = pos.knights<Black>();
  Bitboard bpawns = pos.pawns<Black>();
  while (bknight) {
    Square sq = poplsb(bknight);
    int rr = 7 - rankof(sq);

    // Check outposts on relative ranks 3 to 6
    if (rr >= RANK_3 && rr <= RANK_6) {
      if (Attacks::GetPawnAttacks(White, sq) & bpawns) {
        // Must be supported by a pawn
        evalData.subtract(KNIGHT_OUTPOST_BONUS[rr]);
      }
    }
  }
}
int Evaluator::CalculateGamePhase(const Position &pos) const {
  return std::clamp(pos.getPhase(), 0, 24);
}

Score Evaluator::CalculateFinalScore(const Position &pos) const {
  Score opening = evalData.opening();
  Score endgame = evalData.endgame();

  // Add tempo bonus for side to move
  if (pos.sideToMove() == White) {
    opening += openingScore(TEMPO_BONUS);
    endgame += endgameScore(TEMPO_BONUS);
  } else {
    opening -= openingScore(TEMPO_BONUS);
    endgame -= endgameScore(TEMPO_BONUS);
  }

  int phase = CalculateGamePhase(pos);

  Score finalScore = (opening * phase + endgame * (24 - phase)) / 24;

  Score result = (pos.sideToMove() == White) ? finalScore : -finalScore;

  return result;
}
} // namespace ASTROVE::eval
