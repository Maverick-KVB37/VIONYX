#pragma once

#include "../core/attacks.h"
#include "../core/bitboard.h"
#include "../core/move.h"
#include "../core/types.h"
#include "../core/zobrist.h"
#include <cassert>
#include <string>
#include <vector>

const std::string defaultFEN =
    "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
// Data members are ordered to improve memory layout and cache locality while
// minimizing implicit padding. No explicit padding is introduced to avoid
// unnecessary object size growth.
class StateInfo {
public:
  // 8-byte types packed at the top
  uint64_t hashKey;
  uint64_t pawnKey;
  Bitboard checkers;
  Bitboard pinMaskHV;
  Bitboard pinMaskD;
  StateInfo *previous;

  // 4-byte or 1-byte types at the bottom
  Square enpassantSquare;
  Piece captured;
  U8 castlingRights;
  U8 halfMoveClock;

  StateInfo()
      : hashKey(0), pawnKey(0), checkers(EMPTY_BB), pinMaskHV(EMPTY_BB),
        pinMaskD(EMPTY_BB), previous(nullptr), enpassantSquare(NO_SQ),
        captured(None), castlingRights(NO_CASTLING), halfMoveClock(0) {}
};

class Position {
public:
  Position(const std::string &FEN = defaultFEN);
  void parseFEN(const std::string &FEN);
  std::string toFEN() const;
  void print();
  void PrintAllBitboards() const;

  template <Color c> void makemove(Move move);
  template <Color c> void unmakemove(Move move);

  inline Piece pieceAt(Square sq) const { return board[sq]; }
  inline Color sideToMove() const { return stm; }
  inline uint64_t hash() const { return state->hashKey; }
  inline Square epSquare() const { return state->enpassantSquare; }
  inline U8 castling() const { return state->castlingRights; }

  template <Color c> inline bool isSquareAttacked(Square sq) const;
  template <Color c> Square kingsq() const;
  template <Color c> bool inCheck() const;

  template <Color c> constexpr Bitboard pawns() const {
    return (c == White) ? PiecesBB[WhitePawn] : PiecesBB[BlackPawn];
  }
  template <Color c> constexpr Bitboard knights() const {
    return (c == White) ? PiecesBB[WhiteKnight] : PiecesBB[BlackKnight];
  }
  template <Color c> constexpr Bitboard bishops() const {
    return (c == White) ? PiecesBB[WhiteBishop] : PiecesBB[BlackBishop];
  }
  template <Color c> constexpr Bitboard rooks() const {
    return (c == White) ? PiecesBB[WhiteRook] : PiecesBB[BlackRook];
  }
  template <Color c> constexpr Bitboard queens() const {
    return (c == White) ? PiecesBB[WhiteQueen] : PiecesBB[BlackQueen];
  }
  template <Color c> constexpr Bitboard kings() const {
    return (c == White) ? PiecesBB[WhiteKing] : PiecesBB[BlackKing];
  }

  inline Bitboard occupancy(Color c) const {
    return c == White ? occupancyWhite : occupancyBlack;
  }
  inline Bitboard occupancy() const { return occupancyAll; }

  uint64_t generateHashKey() const;
  uint64_t generatePawnKey() const;

  inline bool isDrawByRepetition(int ply) const;
  inline bool isDrawByFiftyMove() const;

  Bitboard getPiecesBB(int index) const { return PiecesBB[index]; }
  Bitboard pieces(Color c, PieceType pt) const { return PiecesBB[c * 6 + pt]; }

  template <Color c> inline bool hasNonPawnMaterial() const {
    return (knights<c>() | bishops<c>() | rooks<c>() | queens<c>()) != 0ULL;
  }

  template <Color c> inline void makeNullMove();

  template <Color c> inline void unmakeNullMove();

  inline uint8_t getHalfMoveClock() const { return state->halfMoveClock; }
  inline uint16_t getFullMoves() const { return fullMoveCounter; }

private:
  Bitboard PiecesBB[12];
  Piece board[64];

  Bitboard occupancyWhite;
  Bitboard occupancyBlack;
  Bitboard occupancyAll;

  Color stm;
  StateInfo *state;

  StateInfo stateStack[1024];
  uint16_t stateCount;

  uint64_t positionHistory[1024];
  int historyCount = 0;
  uint16_t fullMoveCounter;

  int mgScore = 0;
  int egScore = 0;
  int gamePhase = 0;

public:
  inline int getMgScore() const { return mgScore; }
  inline int getEgScore() const { return egScore; }
  inline int getPhase() const { return gamePhase; }
  inline uint64_t getPawnKey() const { return state->pawnKey; }

private:
  void placePiece(Piece piece, Square sq);
  void removePiece(Square sq);
  void movePiece(Square from, Square to);

  inline void togglePiece(Piece piece, Square sq) {
    state->hashKey ^= zobrist.pieceKeys[piece][sq];
    if (piece == WhitePawn || piece == BlackPawn) {
      state->pawnKey ^= zobrist.pieceKeys[piece][sq];
    }
  }

  inline void toggleEnpassant(Square sq) {
    if (sq != NO_SQ) {
      state->hashKey ^= zobrist.enpassantKeys[sq & 7];
    }
  }

  inline void toggleCastling(U8 rights) {
    state->hashKey ^= zobrist.castlingKeys[rights];
  }

  inline void toggleSide() { state->hashKey ^= zobrist.sideKey; }
};

template <Color c> inline bool Position::isSquareAttacked(Square sq) const {
  if (sq == NO_SQ)
    return false;

  Bitboard pawnAtks = Attacks::GetPawnAttacks(~c, sq);
  if (pawnAtks & pawns<c>())
    return true;

  Bitboard knightAtks = Attacks::GetKnightAttacks(sq);
  if (knightAtks & knights<c>())
    return true;

  Bitboard kingAtks = Attacks::GetKingAttacks(sq);
  if (kingAtks & kings<c>())
    return true;

  Bitboard occ = occupancyAll;

  Bitboard diagSliders = bishops<c>() | queens<c>();
  if (diagSliders) {
    if (Attacks::GetBishopAttacks(sq, occ) & diagSliders)
      return true;
  }

  Bitboard orthSliders = rooks<c>() | queens<c>();
  if (orthSliders) {
    if (Attacks::GetRookAttacks(sq, occ) & orthSliders)
      return true;
  }

  return false;
}

template <Color c> bool Position::inCheck() const {
  Square kingSq = getlsb(kings<c>());
  return isSquareAttacked<~c>(kingSq);
}

template <Color c> Square Position::kingsq() const {
  if constexpr (c == White)
    return bsf(kings<White>());
  return bsf(kings<Black>());
}

template <Color c> void Position::makemove(Move move) {
  Square from = move.from();
  Square to = move.to();

  Piece movingpiece = board[from];
  Piece capturedPiece = board[to];

  MoveFlag flag = move.flag();

  // setup new state
  assert(stateCount < 1024);
  StateInfo *newState = &stateStack[stateCount++];
  newState->previous = state;

  newState->hashKey = state->hashKey;
  newState->pawnKey = state->pawnKey;
  newState->castlingRights = state->castlingRights;
  newState->enpassantSquare = state->enpassantSquare;
  newState->halfMoveClock = state->halfMoveClock;
  newState->captured = capturedPiece;

  state = newState;

  // remove old ep from hash
  if (state->previous->enpassantSquare != NO_SQ) {
    toggleEnpassant(state->previous->enpassantSquare);
  }
  state->enpassantSquare = NO_SQ;

  state->halfMoveClock++;

  // handle captures (normal)
  if (move.IsCapture() && flag != EnPassant) {
    state->halfMoveClock = 0;
    togglePiece(capturedPiece, to);
    removePiece(to);
  }

  // handle enpassant capture
  if (flag == EnPassant) {
    state->halfMoveClock = 0;
    U8 offset = (c == White) ? -8 : +8;
    Square capSq = Square(to + offset);
    Piece epPawn = makepiece<~c>(Pawn);

    togglePiece(epPawn, capSq);
    removePiece(capSq);
  }

  // handle double pawn push
  if (flag == DoublePawnPush) {
    state->halfMoveClock = 0;
    U8 offset = (c == White) ? -8 : 8;
    Square epSq = Square(to + offset);

    Bitboard enemyPawns = pawns<~c>();
    Bitboard epAttackers = Attacks::GetPawnAttacks(c, epSq);
    if (epAttackers & enemyPawns) {
      state->enpassantSquare = epSq;
      toggleEnpassant(epSq);
    }
  }

  U8 oldCastling = state->castlingRights;

  // handle castling
  if (flag == KingCastle) {
    if constexpr (c == White) {
      togglePiece(WhiteKing, from);
      removePiece(from);
      togglePiece(WhiteKing, to);
      placePiece(WhiteKing, to);

      togglePiece(WhiteRook, SQ_H1);
      removePiece(SQ_H1);
      togglePiece(WhiteRook, SQ_F1);
      placePiece(WhiteRook, SQ_F1);

      state->castlingRights &= ~(WHITE_OO | WHITE_OOO);
    } else {
      togglePiece(BlackKing, from);
      removePiece(from);
      togglePiece(BlackKing, to);
      placePiece(BlackKing, to);

      togglePiece(BlackRook, SQ_H8);
      removePiece(SQ_H8);
      togglePiece(BlackRook, SQ_F8);
      placePiece(BlackRook, SQ_F8);

      state->castlingRights &= ~(BLACK_OO | BLACK_OOO);
    }
  } else if (flag == QueenCastle) {
    if constexpr (c == White) {
      togglePiece(WhiteKing, from);
      removePiece(from);
      togglePiece(WhiteKing, to);
      placePiece(WhiteKing, to);

      togglePiece(WhiteRook, SQ_A1);
      removePiece(SQ_A1);
      togglePiece(WhiteRook, SQ_D1);
      placePiece(WhiteRook, SQ_D1);

      state->castlingRights &= ~(WHITE_OO | WHITE_OOO);
    } else {
      togglePiece(BlackKing, from);
      removePiece(from);
      togglePiece(BlackKing, to);
      placePiece(BlackKing, to);

      togglePiece(BlackRook, SQ_A8);
      removePiece(SQ_A8);
      togglePiece(BlackRook, SQ_D8);
      placePiece(BlackRook, SQ_D8);

      state->castlingRights &= ~(BLACK_OO | BLACK_OOO);
    }
  }

  // handle promotion
  else if (move.IsPromotion()) {
    state->halfMoveClock = 0;

    Piece promotedPiece;
    switch (flag) {
    case KnightPromotion:
    case KnightPromoCapture:
      promotedPiece = makepiece<c>(Knight);
      break;
    case BishopPromotion:
    case BishopPromoCapture:
      promotedPiece = makepiece<c>(Bishop);
      break;
    case RookPromotion:
    case RookPromoCapture:
      promotedPiece = makepiece<c>(Rook);
      break;
    default:
      promotedPiece = makepiece<c>(Queen);
      break;
    }
    Piece pawn = makepiece<c>(Pawn);
    togglePiece(pawn, from);
    removePiece(from);

    togglePiece(promotedPiece, to);
    placePiece(promotedPiece, to);
  }
  // normal move
  else if (flag != KingCastle && flag != QueenCastle) {
    if (piecetype(movingpiece) == Pawn) {
      state->halfMoveClock = 0;
    }

    togglePiece(movingpiece, from);
    removePiece(from);
    togglePiece(movingpiece, to);
    placePiece(movingpiece, to);
  }

  // update castling rights if rook or king moved
  if (flag != KingCastle && flag != QueenCastle) {
    PieceType movedType = piecetype(movingpiece);

    if (movedType == King) {
      if constexpr (c == White) {
        state->castlingRights &= ~(WHITE_OO | WHITE_OOO);
      } else {
        state->castlingRights &= ~(BLACK_OO | BLACK_OOO);
      }
    } else if (movedType == Rook) {
      if constexpr (c == White) {
        if (from == SQ_A1)
          state->castlingRights &= ~WHITE_OOO;
        else if (from == SQ_H1)
          state->castlingRights &= ~WHITE_OO;
      } else {
        if (from == SQ_A8)
          state->castlingRights &= ~BLACK_OOO;
        else if (from == SQ_H8)
          state->castlingRights &= ~BLACK_OO;
      }
    }
  }

  // rook capture revokes castling
  if (to == SQ_A1)
    state->castlingRights &= ~WHITE_OOO;
  else if (to == SQ_H1)
    state->castlingRights &= ~WHITE_OO;
  else if (to == SQ_A8)
    state->castlingRights &= ~BLACK_OOO;
  else if (to == SQ_H8)
    state->castlingRights &= ~BLACK_OO;

  if (oldCastling != state->castlingRights) {
    toggleCastling(oldCastling);
    toggleCastling(state->castlingRights);
  }

  stm = ~stm;
  toggleSide();

  positionHistory[historyCount++] = state->hashKey;

  if (stm == White) {
    fullMoveCounter++;
  }
}

template <Color c> void Position::unmakemove(Move move) {
  Square from = move.from();
  Square to = move.to();
  MoveFlag flag = move.flag();

  stm = ~stm;

  if (stm == Black) {
    fullMoveCounter--;
  }

  Piece capturedPiece = state->captured;

  if (flag == KingCastle) {
    if constexpr (c == White) {
      movePiece(to, from);
      movePiece(SQ_F1, SQ_H1);
    } else {
      movePiece(to, from);
      movePiece(SQ_F8, SQ_H8);
    }
  } else if (flag == QueenCastle) {
    if constexpr (c == White) {
      movePiece(to, from);
      movePiece(SQ_D1, SQ_A1);
    } else {
      movePiece(to, from);
      movePiece(SQ_D8, SQ_A8);
    }
  }

  else if (move.IsPromotion()) {
    removePiece(to);
    placePiece(makepiece<c>(Pawn), from);

    if (capturedPiece != None) {
      placePiece(capturedPiece, to);
    }
  }
  // enpassant
  else if (flag == EnPassant) {
    movePiece(to, from);
    int8_t offset = (c == White) ? -8 : 8;
    Square capSq = Square(to + offset);
    Piece epPawn = makepiece<~c>(Pawn);
    placePiece(epPawn, capSq);
  }
  // normal move
  else {
    Piece movingPiece = board[to];

    removePiece(to);
    placePiece(movingPiece, from);

    if (capturedPiece != None) {
      placePiece(capturedPiece, to);
    }
  }

  state = state->previous;
  stateCount--;

  historyCount--;
}

inline bool Position::isDrawByRepetition(int ply) const {
  if (historyCount < 4)
    return false;

  int irreversibleMovePly =
      historyCount - 1 - static_cast<int>(state->halfMoveClock);
  if (irreversibleMovePly < 0)
    irreversibleMovePly = 0;

  int repetitions = 0;
  for (int i = historyCount - 3; i >= irreversibleMovePly; i -= 2) {
    if (positionHistory[i] == state->hashKey) {
      repetitions++;
      // twofold in search, threefold at root
      if (ply > 0 && repetitions >= 1)
        return true;
      if (ply == 0 && repetitions >= 2)
        return true;
    }
  }
  return false;
}

inline bool Position::isDrawByFiftyMove() const {
  if (state->halfMoveClock < 4)
    return false;
  return state->halfMoveClock >= 100;
}

template <Color c> inline void Position::makeNullMove() {
  if (stateCount >= 1023)
    return;

  StateInfo *newState = &stateStack[stateCount++];
  *newState = *state;
  newState->previous = state;
  state = newState;

  state->checkers = 0ULL;
  state->pinMaskHV = 0ULL;
  state->pinMaskD = 0ULL;

  if (state->enpassantSquare != NO_SQ) {
    toggleEnpassant(state->enpassantSquare);
    state->enpassantSquare = NO_SQ;
  }

  state->halfMoveClock++;
  if (c == Black) {
    fullMoveCounter++;
  }

  stm = ~c;
  toggleSide();

  positionHistory[historyCount++] = state->hashKey;
}

template <Color c> inline void Position::unmakeNullMove() {

  if (historyCount > 0) {
    historyCount--;
  }

  if (state->previous == nullptr)
    return;

  state = state->previous;
  stateCount--;

  stm = c;

  if (c == Black) {
    fullMoveCounter--;
  }
}
