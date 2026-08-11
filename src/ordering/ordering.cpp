#include "ordering.h"
#include "../core/attacks.h"
#include <algorithm>
#include <cstring>

MoveOrderer::MoveOrderer() {
  for (int attacker = Pawn; attacker <= King; ++attacker) {
    for (int victim = Pawn; victim <= King; ++victim) {
      mvv_lva[attacker][victim] =
          SEEVALUE[victim] + 6 - (SEEVALUE[attacker] / 100);
    }
  }
}

Bitboard MoveOrderer::attackersForSide(const Position &pos, Color attackerColor,
                                       Square sq, Bitboard occupiedBB) {
  Bitboard attackingBishops = pos.getPiecesBB(makepiece(attackerColor, Bishop));
  Bitboard attackingRooks = pos.getPiecesBB(makepiece(attackerColor, Rook));
  Bitboard attackingQueens = pos.getPiecesBB(makepiece(attackerColor, Queen));
  Bitboard attackingKnights = pos.getPiecesBB(makepiece(attackerColor, Knight));
  Bitboard attackingKing = pos.getPiecesBB(makepiece(attackerColor, King));
  Bitboard attackingPawns = pos.getPiecesBB(makepiece(attackerColor, Pawn));

  Bitboard diagonalAttacks = Attacks::GetBishopAttacks(sq, occupiedBB);
  Bitboard orthogonalAttacks = Attacks::GetRookAttacks(sq, occupiedBB);

  Bitboard attackers =
      (diagonalAttacks & (attackingBishops | attackingQueens)) |
      (orthogonalAttacks & (attackingRooks | attackingQueens)) |
      (Attacks::GetKnightAttacks(sq) & attackingKnights) |
      (Attacks::GetKingAttacks(sq) & attackingKing) |
      (Attacks::GetPawnAttacks(~attackerColor, sq) & attackingPawns);

  return attackers;
}

Bitboard MoveOrderer::considerXRays(const Position &pos, Square sq,
                                    Bitboard occupiedBB) {
  Bitboard attackingBishops = pos.bishops<White>() | pos.bishops<Black>();
  Bitboard attackingRooks = pos.rooks<White>() | pos.rooks<Black>();
  Bitboard attackingQueens = pos.queens<White>() | pos.queens<Black>();

  Bitboard diagonalAttacks = Attacks::GetBishopAttacks(sq, occupiedBB);
  Bitboard orthogonalAttacks = Attacks::GetRookAttacks(sq, occupiedBB);

  Bitboard attackers =
      (diagonalAttacks & (attackingBishops | attackingQueens)) |
      (orthogonalAttacks & (attackingRooks | attackingQueens));
  return attackers;
}

Bitboard MoveOrderer::allAttackers(const Position &pos, Square sq,
                                   Bitboard occupiedBB) {
  return attackersForSide(pos, White, sq, occupiedBB) |
         attackersForSide(pos, Black, sq, occupiedBB);
}

Bitboard MoveOrderer::minAttacker(const Position &pos, Bitboard attadef,
                                  Color color, PieceType &attacker) {
  for (attacker = Pawn; attacker <= King; attacker = PieceType(attacker + 1)) {
    Bitboard subset = attadef & pos.getPiecesBB(makepiece(color, attacker));
    if (subset != 0)
      return (subset & -subset);
  }
  return 0ULL;
}

int MoveOrderer::see(const Position &pos, Move move) {
  Square fromSq = move.from();
  Square toSq = move.to();
  PieceType target = piecetype(pos.pieceAt(toSq));
  PieceType attacker = piecetype(pos.pieceAt(fromSq));

  //Handle Enpassant Move
  if(move.IsEnpassant()){
    target = Pawn;
  }
  
  //Target Is Nonetype
  if(target == Nonetype){
    return 0;
  }
  Color sideToMove = ~pos.sideToMove();

  int gain[64] = {0};
  int depth = 0;

  Bitboard occupiedBB = pos.occupancy(White) | pos.occupancy(Black);
  Bitboard attackerBB = SQUAREBB[fromSq];

  Bitboard attadef = allAttackers(pos, toSq, occupiedBB);
  Bitboard maxXray = occupiedBB & ~(pos.knights<White>() | pos.kings<White>() |
                                    pos.knights<Black>() | pos.kings<Black>());

  gain[depth] = SEEVALUE[target];

  while (attackerBB != 0) {
    depth++;
    if (depth >= 64) {
      break;
    }
    gain[depth] = SEEVALUE[attacker] - gain[depth - 1];
    if (std::max(-gain[depth - 1], gain[depth]) < 0)
      break;

    attadef &= ~attackerBB;
    occupiedBB &= ~attackerBB;
    if ((attackerBB & maxXray) != 0) {
      attadef |= considerXRays(pos, toSq, occupiedBB);
    }

    attackerBB = minAttacker(pos, attadef, sideToMove, attacker);
    sideToMove = ~sideToMove;
  }

  for (depth--; depth > 0; depth--) {
    gain[depth - 1] = -std::max(-gain[depth - 1], gain[depth]);
  }

  return gain[0];
}

void MoveOrderer::ScoreMoves(const Position &pos, MoveList &moves, Move ttMove,
                             Move killers[2], const int history[2][64][64],
                             Move prevMove,
                             const Move counterMoves[2][64][64]) {

  /*
  | Counter Move Heuristic                                                      | 
  | Checks if the previous move has a stored counter move that might be strong  | 
  | in this position.                                                           |
  */
  Move counterMove = NO_MOVE;
  if (prevMove != NO_MOVE) {
    counterMove =
        counterMoves[pos.sideToMove()][prevMove.from()][prevMove.to()];
  }

  for (int i = 0; i < moves.size(); i++) {
    const Move &move = moves[i];

    /*
    | Transposition Table Move                                                    | 
    | The move retrieved from the TT is the most likely to be the best move.      | 
    | It gets the highest score to be searched first.                             |
    */
    if (move == ttMove) {
      moves.scores[i] = SCORE_TT_MOVE;
    }

    /*
    | Captures (MVV-LVA)                                                          | 
    | Most Valuable Victim - Least Valuable Attacker. Good captures are scored    | 
    | highly. En passant captures are also handled here.                          |
    */
    else if (move.IsCapture()) {
      PieceType attacker = piecetype(pos.pieceAt(move.from()));
      PieceType victim = piecetype(pos.pieceAt(move.to()));

      // En passant
      if (move.IsEnpassant()) {
        victim = Pawn;
      }
      if (victim != Nonetype) {
        moves.scores[i] = SCORE_CAPTURE_BASE + mvv_lva[attacker][victim];
      } else {
        moves.scores[i] = SCORE_CAPTURE_BASE;
      }
    }

    /*
    | Promotions                                                                  | 
    | Non-capture promotions are very valuable, especially Queen promotions,      | 
    | so they are scored highly just below good captures.                         |
    */
    else if (move.IsPromotion()) {
      moves.scores[i] = SCORE_CAPTURE_BASE + 600;
    }

    /*
    | Killer Moves                                                                | 
    | Quiet moves that caused a beta cutoff at the same ply in other branches     | 
    | of the search tree.                                                         |
    */
    else if (move == killers[0]) {
      moves.scores[i] = SCORE_KILLER_1;
    } else if (move == killers[1]) {
      moves.scores[i] = SCORE_KILLER_2;
    }

    else if (move == counterMove) {
      moves.scores[i] = 17000;
    }

    /*
    | History Heuristic                                                          | 
    | Orders quiet moves based on how often they havecaused beta cutoffs in      | 
    | the past, helping us find good quiet movesfaster.                          |
    */
    else {
      int side = pos.sideToMove();
      int hscore = history[side][move.from()][move.to()];
      // Ensure history score does not exceed killer moves
      if (hscore > 16000)
        hscore = 16000;
      moves.scores[i] = hscore;
    }
  }


}

Move MoveOrderer::PickNextMove(MoveList &moves, int startIndex) {
  int bestIdx = startIndex;
  for (int j = startIndex + 1; j < moves.size(); ++j) {
    if (moves.scores[j] > moves.scores[bestIdx]) {
      bestIdx = j;
    }
  }
  if (bestIdx != startIndex) {
    std::swap(moves[startIndex], moves[bestIdx]);
    std::swap(moves.scores[startIndex], moves.scores[bestIdx]);
  }
  return moves[startIndex];
}

void MoveOrderer::ScoreCaptures(const Position &pos, MoveList &captures,
                                Move ttMove) {
  for (int i = 0; i < captures.size(); i++) {
    const Move &move = captures[i];
    if (move == ttMove) {
      captures.scores[i] = 1000000;
    } else {
      captures.scores[i] = see(pos, move);
    }
  }
}
