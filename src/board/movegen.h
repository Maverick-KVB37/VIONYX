#pragma once
#include "../core/attacks.h"
#include "../core/move.h"
#include "../core/types.h"
#include "position.h"
#include <vector>

class MoveGenerator {
public:
  template <Color c>
  void GenerateAllMoves(const Position &pos, MoveList &moves) {
    moves.clear();

    GeneratePawnMoves<c>(pos, moves);
    GenerateKnightMoves<c>(pos, moves);
    GenerateKingMoves<c>(pos, moves);
    GenerateSlidingMoves<c>(pos, moves);
    GenerateCastlingMoves<c>(pos, moves);
  }

  /*
  | Quiescence Moves | | Generates only capture and promotion moves, which are
  used primarily in     | | the quiescence search to resolve noisy positions. |
  */
  template <Color c>
  void GenerateCaptures(const Position &pos, MoveList &moves) {
    moves.clear();

    GeneratePawnCaptures<c>(pos, moves);
    GenerateKnightCaptures<c>(pos, moves);
    GenerateKingCaptures<c>(pos, moves);
    GenerateSlidingCaptures<c>(pos, moves);
  }

private:
  template <Color c>
  void GenerateKnightMoves(const Position &pos, MoveList &moves) {
    Bitboard knights = pos.knights<c>();
    Bitboard friendly = pos.occupancy(c);
    Bitboard enemy = pos.occupancy(~c);

    while (knights) {
      Square from = poplsb(knights);

      Bitboard attacks = Attacks::GetKnightAttacks(from);
      attacks &= ~friendly;

      Bitboard captures = attacks & enemy;
      Bitboard quiets = attacks & ~enemy;

      while (captures) {
        Square to = poplsb(captures);
        moves.Add(Move(from, to, Capture));
      }
      while (quiets) {
        Square to = poplsb(quiets);
        moves.Add(Move(from, to, QuietMove));
      }
    }
  }

  template <Color c>
  void GenerateSlidingMoves(const Position &pos, MoveList &moves) {
    const Bitboard occupancy = pos.occupancy();
    const Bitboard friendly = pos.occupancy(c);
    const Bitboard enemy = pos.occupancy(~c);

    auto generate_for_slider = [&](Bitboard pieces, auto get_attacks_func) {
      while (pieces) {
        Square from = poplsb(pieces);
        Bitboard attacks = get_attacks_func(from, occupancy) & ~friendly;

        Bitboard captures = attacks & enemy;
        while (captures) {
          moves.Add(Move(from, poplsb(captures), Capture));
        }

        Bitboard quiets = attacks & ~enemy;
        while (quiets) {
          moves.Add(Move(from, poplsb(quiets), QuietMove));
        }
      }
    };

    generate_for_slider(pos.bishops<c>(), Attacks::GetBishopAttacks);
    generate_for_slider(pos.rooks<c>(), Attacks::GetRookAttacks);
    generate_for_slider(pos.queens<c>(), Attacks::GetQueenAttacks);
  }

  template <Color c>
  void GenerateKingMoves(const Position &pos, MoveList &moves) {
    Bitboard kings = pos.kings<c>();
    Bitboard friendly = pos.occupancy(c);
    Bitboard enemy = pos.occupancy(~c);

    while (kings) {
      Square from = poplsb(kings);

      Bitboard attacks = Attacks::GetKingAttacks(from);
      attacks &= ~friendly;

      Bitboard captures = attacks & enemy;
      Bitboard quiets = attacks & ~enemy;

      while (captures) {
        Square to = poplsb(captures);
        moves.Add(Move(from, to, Capture));
      }
      while (quiets) {
        Square to = poplsb(quiets);
        moves.Add(Move(from, to, QuietMove));
      }
    }
  }

  template <Color c>
  void GenerateCastlingMoves(const Position &pos, MoveList &moves) {
    if constexpr (c == White) {
      if (pos.castling() & WHITE_OO) {
        if (pos.pieceAt(SQ_F1) == None && pos.pieceAt(SQ_G1) == None &&
            !pos.isSquareAttacked<Black>(SQ_E1) &&
            !pos.isSquareAttacked<Black>(SQ_F1) &&
            !pos.isSquareAttacked<Black>(SQ_G1)) {
          moves.Add(Move(SQ_E1, SQ_G1, KingCastle));
        }
      }
      if (pos.castling() & WHITE_OOO) {
        if (pos.pieceAt(SQ_D1) == None && pos.pieceAt(SQ_C1) == None &&
            pos.pieceAt(SQ_B1) == None && !pos.isSquareAttacked<Black>(SQ_E1) &&
            !pos.isSquareAttacked<Black>(SQ_D1) &&
            !pos.isSquareAttacked<Black>(SQ_C1)) {
          moves.Add(Move(SQ_E1, SQ_C1, QueenCastle));
        }
      }
    } else {
      if (pos.castling() & BLACK_OO) {
        if (pos.pieceAt(SQ_F8) == None && pos.pieceAt(SQ_G8) == None &&
            !pos.isSquareAttacked<White>(SQ_E8) &&
            !pos.isSquareAttacked<White>(SQ_F8) &&
            !pos.isSquareAttacked<White>(SQ_G8)) {
          moves.Add(Move(SQ_E8, SQ_G8, KingCastle));
        }
      }
      if (pos.castling() & BLACK_OOO) {
        if (pos.pieceAt(SQ_D8) == None && pos.pieceAt(SQ_C8) == None &&
            pos.pieceAt(SQ_B8) == None && !pos.isSquareAttacked<White>(SQ_E8) &&
            !pos.isSquareAttacked<White>(SQ_D8) &&
            !pos.isSquareAttacked<White>(SQ_C8)) {
          moves.Add(Move(SQ_E8, SQ_C8, QueenCastle));
        }
      }
    }
  }

  template <Color c>
  void GeneratePawnMoves(const Position &pos, MoveList &moves) {
    Bitboard pawns = pos.pawns<c>();
    Bitboard empty = ~pos.occupancy();
    Bitboard enemy = pos.occupancy(~c);

    int forward = (c == White) ? 8 : -8;
    int doublepawnpush = (c == White) ? 1 : 6;
    int promotionrank = (c == White) ? 6 : 1;

    while (pawns) {
      Square from = poplsb(pawns);
      int fromrank = from / 8;
      Square to = Square(from + forward);

      /*
      | Single Pawn Push | | Generates moves for a pawn advancing one square
      forward. Handles promotions | | if the pawn reaches the promotion rank. |
      */
      if (empty & (1ULL << to)) {
        if (fromrank == promotionrank) {
          moves.Add(Move(from, to, QueenPromotion));
          moves.Add(Move(from, to, RookPromotion));
          moves.Add(Move(from, to, BishopPromotion));
          moves.Add(Move(from, to, KnightPromotion));
        } else {
          moves.Add(Move(from, to, QuietMove));
          /*
          | Double Pawn Push | | Generates a double push if the pawn is on its
          starting rank and the         | | squares in front are clear. |
          */
          if (fromrank == doublepawnpush) {
            Square to2 = Square(from + 2 * forward);
            if ((empty & (1ULL << to2)) && (empty & (1ULL << to))) {
              moves.Add(Move(from, to2, DoublePawnPush));
            }
          }
        }
      }

      /*
      | Pawn Captures | | Generates all valid pawn captures, including
      promotions via capture.        |
      */
      Bitboard attacks = Attacks::GetPawnAttacks(c, from) & enemy;
      while (attacks) {
        Square capSq = poplsb(attacks);
        if (fromrank == promotionrank) {
          moves.Add(Move(from, capSq, QueenPromoCapture));
          moves.Add(Move(from, capSq, RookPromoCapture));
          moves.Add(Move(from, capSq, BishopPromoCapture));
          moves.Add(Move(from, capSq, KnightPromoCapture));
        } else {
          moves.Add(Move(from, capSq, Capture));
        }
      }

      /*
      | En Passant Captures | | Generates en passant captures if available in
      the current position.         |
      */
      Square epSq = pos.epSquare();
      if (epSq != NO_SQ) {
        Bitboard epAttack = Attacks::GetPawnAttacks(c, from) & (1ULL << epSq);
        if (epAttack) {
          moves.Add(Move(from, epSq, EnPassant));
        }
      }
    }
  }

  // --- capture-only generators ---

  template <Color c>
  void GenerateKnightCaptures(const Position &pos, MoveList &moves) {
    Bitboard knights = pos.knights<c>();
    Bitboard enemy = pos.occupancy(~c);

    while (knights) {
      Square from = poplsb(knights);
      Bitboard captures = Attacks::GetKnightAttacks(from) & enemy;
      while (captures) {
        Square to = poplsb(captures);
        moves.Add(Move(from, to, Capture));
      }
    }
  }

  template <Color c>
  void GenerateKingCaptures(const Position &pos, MoveList &moves) {
    Bitboard kings = pos.kings<c>();
    Bitboard enemy = pos.occupancy(~c);

    while (kings) {
      Square from = poplsb(kings);
      Bitboard captures = Attacks::GetKingAttacks(from) & enemy;
      while (captures) {
        Square to = poplsb(captures);
        moves.Add(Move(from, to, Capture));
      }
    }
  }

  template <Color c>
  void GenerateSlidingCaptures(const Position &pos, MoveList &moves) {
    const Bitboard occupancy = pos.occupancy();
    const Bitboard friendly = pos.occupancy(c);
    const Bitboard enemy = pos.occupancy(~c);

    auto generate_for_slider = [&](Bitboard pieces, auto get_attacks_func) {
      while (pieces) {
        Square from = poplsb(pieces);
        Bitboard captures =
            get_attacks_func(from, occupancy) & ~friendly & enemy;
        while (captures) {
          moves.Add(Move(from, poplsb(captures), Capture));
        }
      }
    };

    generate_for_slider(pos.bishops<c>(), Attacks::GetBishopAttacks);
    generate_for_slider(pos.rooks<c>(), Attacks::GetRookAttacks);
    generate_for_slider(pos.queens<c>(), Attacks::GetQueenAttacks);
  }

  template <Color c>
  void GeneratePawnCaptures(const Position &pos, MoveList &moves) {
    Bitboard pawns = pos.pawns<c>();
    Bitboard enemy = pos.occupancy(~c);
    Bitboard empty = ~pos.occupancy();

    int forward = (c == White) ? 8 : -8;
    int promotionrank = (c == White) ? 6 : 1;

    while (pawns) {
      Square from = poplsb(pawns);
      int fromrank = from / 8;

      /*
      | Promotion Pushes | | Treated as captures because they significantly
      alter the material balance   | | and must be evaluated during quiescence
      search.                             |
      */
      if (fromrank == promotionrank) {
        Square to = Square(from + forward);
        if (empty & (1ULL << to)) {
          moves.Add(Move(from, to, QueenPromotion));
          moves.Add(Move(from, to, RookPromotion));
          moves.Add(Move(from, to, BishopPromotion));
          moves.Add(Move(from, to, KnightPromotion));
        }
      }

      Bitboard attacks = Attacks::GetPawnAttacks(c, from) & enemy;
      while (attacks) {
        Square capSq = poplsb(attacks);
        if (fromrank == promotionrank) {
          moves.Add(Move(from, capSq, QueenPromoCapture));
          moves.Add(Move(from, capSq, RookPromoCapture));
          moves.Add(Move(from, capSq, BishopPromoCapture));
          moves.Add(Move(from, capSq, KnightPromoCapture));
        } else {
          moves.Add(Move(from, capSq, Capture));
        }
      }

      Square epSq = pos.epSquare();
      if (epSq != NO_SQ) {
        Bitboard epAttack = Attacks::GetPawnAttacks(c, from) & (1ULL << epSq);
        if (epAttack) {
          moves.Add(Move(from, epSq, EnPassant));
        }
      }
    }
  }
};
