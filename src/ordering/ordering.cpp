#include "ordering.h"
#include "../core/attacks.h"
#include <cstring>
#include <algorithm>


// Returns the bitboard of attackers of given color on square `sq`
Bitboard MoveOrderer::attackersForSide(const Position& pos, Color attackerColor, Square sq, Bitboard occupiedBB) {
    Bitboard attackingBishops = pos.getPiecesBB(makepiece(attackerColor, Bishop));
    Bitboard attackingRooks   = pos.getPiecesBB(makepiece(attackerColor, Rook));
    Bitboard attackingQueens  = pos.getPiecesBB(makepiece(attackerColor, Queen));
    Bitboard attackingKnights = pos.getPiecesBB(makepiece(attackerColor, Knight));
    Bitboard attackingKing    = pos.getPiecesBB(makepiece(attackerColor, King));
    Bitboard attackingPawns   = pos.getPiecesBB(makepiece(attackerColor, Pawn));

    Bitboard diagonalAttacks = Attacks::get_bishop_attacks(sq, occupiedBB);
    Bitboard orthogonalAttacks = Attacks::get_rook_attacks(sq, occupiedBB);

    Bitboard attackers = (diagonalAttacks & (attackingBishops | attackingQueens))
                       | (orthogonalAttacks & (attackingRooks | attackingQueens))
                       | (Attacks::get_knight_attacks(sq) & attackingKnights)
                       | (Attacks::get_king_attacks(sq) & attackingKing)
                       | (Attacks::get_pawn_attacks(~attackerColor, sq) & attackingPawns);

    return attackers;
}

// Returns xrays attackers on sq (excluding knights/kings)
Bitboard MoveOrderer::considerXRays(const Position& pos, Square sq, Bitboard occupiedBB) {
    Bitboard attackingBishops = pos.bishops<White>() | pos.bishops<Black>();
    Bitboard attackingRooks   = pos.rooks<White>() | pos.rooks<Black>();
    Bitboard attackingQueens  = pos.queens<White>() | pos.queens<Black>();

    Bitboard diagonalAttacks = Attacks::get_bishop_attacks(sq, occupiedBB);
    Bitboard orthogonalAttacks = Attacks::get_rook_attacks(sq, occupiedBB);

    Bitboard attackers = (diagonalAttacks & (attackingBishops | attackingQueens))
                       | (orthogonalAttacks & (attackingRooks | attackingQueens));
    return attackers;
}

// Return all attackers on square sq by any side.
Bitboard MoveOrderer::allAttackers(const Position& pos, Square sq, Bitboard occupiedBB) {
    return attackersForSide(pos, White, sq, occupiedBB)
         | attackersForSide(pos, Black, sq, occupiedBB);
}

// Find minimum value attacker bitboard and set attacker to its piece type.
Bitboard MoveOrderer::minAttacker(const Position& pos, Bitboard attadef, Color color, PieceType& attacker) {
    for (attacker = Pawn; attacker <= King; attacker = PieceType(attacker + 1)) {
        Bitboard subset = attadef & pos.getPiecesBB(makepiece(color, attacker));
        if (subset != 0) return (subset & -subset);  // Least significant bit
    }
    return 0ULL;
}

// Static Exchange Evaluation for a move.
int MoveOrderer::see(const Position& pos, Move move) {
    Square fromSq = move.from();
    Square toSq = move.to();
    PieceType target = piecetype(pos.pieceAt(toSq));
    PieceType attacker = piecetype(pos.pieceAt(fromSq));
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

// Quick check if SEE for move is >= threshold
bool MoveOrderer::seeGe(const Position& pos, Move move, int threshold) {
    Square fromSq = move.from();
    Square toSq = move.to();

    PieceType attackerType = piecetype(pos.pieceAt(fromSq));
    PieceType capturedPiece = piecetype(pos.pieceAt(toSq));

    bool isEnpassantMove = (attackerType == Pawn && toSq == pos.epSquare());
    int toRank = rankof(toSq);
    bool isPromotionMove = (attackerType == Pawn && (toRank == RANK_1 || toRank == RANK_8));

    if (isPromotionMove)
        return see(pos, move) >= threshold;

    if (isEnpassantMove)
        capturedPiece = Pawn;

    if (capturedPiece == Nonetype && !isEnpassantMove)
        return threshold <= 0;

    int balance = SEEVALUE[capturedPiece] - threshold;
    if (balance < 0)
        return false;

    balance -= SEEVALUE[attackerType];
    if (balance >= 0)
        return true;

    return see(pos, move) >= threshold;
}

void MoveOrderer::scoreMoves(const Position& pos, MoveList& moves, Move ttMove, Move killers[2]) {
   // std::cerr << "DEBUG: scoreMoves entered, moves.size=" << moves.size() << "\n";

    scores.clear();
    scores.resize(moves.size());
    
    for (size_t i = 0; i < moves.size(); i++) {
        //std::cerr << "DEBUG: Scoring move " << i << "/" << moves.size() << "\n";

        const Move& move = moves[i];
        int score = 0;
        
        if (move.from() == ttMove.from() && move.to() == ttMove.to()) {
            score = 100000;
        } else if (move.from() == killers[0].from() && move.to() == killers[0].to()) {
            score = 90000;
        } else if (move.from() == killers[1].from() && move.to() == killers[1].to()) {
            score = 80000;
        } else if (move.is_capture()) {
            //std::cerr << "DEBUG: Scoring move " << i << "/" << moves.size() << "\n";
            score = see(pos, move) + 10000;
            //std::cerr << "DEBUG: see returned score=" << score << "\n";
        } else {
            score = 0;
        }
        
        scores[i] = score;
    }
    /*
    std::sort(moves.begin(), moves.end(), [&](const Move& a, const Move& b) {
        size_t i = &a - &moves[0];
        size_t j = &b - &moves[0];
        return scores[i] > scores[j];
    });
    */
    
    size_t n = moves.size();
    for (size_t i = 0; i < n && i < 256; i++) {  // Add safety limit
        size_t maxIdx = i;
        for (size_t j = i + 1; j < n && j < 256; j++) {  // Add safety limit
            if (scores[j] > scores[maxIdx]) {
                maxIdx = j;
            }
        }
        if (maxIdx != i) {
            // Safe swap
            Move tempMove = moves[i];
            moves[i] = moves[maxIdx];
            moves[maxIdx] = tempMove;
            
            int tempScore = scores[i];
            scores[i] = scores[maxIdx];
            scores[maxIdx] = tempScore;
        }
    }
    
    //std::cerr << "DEBUG: scoreMoves done\n";

}

void MoveOrderer::scoreCaptures(const Position& pos, MoveList& captures) {
    scores.clear();
    scores.resize(captures.size());
    
    for (size_t i = 0; i < captures.size(); i++) {
        const Move& move = captures[i];
        scores[i] = see(pos, move);
    }
    
    // Sort captures by SEE value
    for (size_t i = 0; i < captures.size(); i++) {
        for (size_t j = i + 1; j < captures.size(); j++) {
            if (scores[j] > scores[i]) {
                std::swap(captures[i], captures[j]);
                std::swap(scores[i], scores[j]);
            }
        }
    }
}

