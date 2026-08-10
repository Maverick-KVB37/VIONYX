#include "position.h"
#include "../core/bitboard.h"
#include "../core/types.h"
#include "../core/zobrist.h"
#include "../evaluation/evaluation.h"
#include <iostream>
#include <sstream>

void print_bitboard(Bitboard bb, const std::string &name) {
  std::cout << "\n--- " << name << " ---\n";
  for (int rank = 7; rank >= 0; --rank) {
    std::cout << rank + 1 << " |";
    for (int file = 0; file < 8; ++file) {
      Square sq = makesquare(File(file), Rank(rank));
      if ((bb >> sq) & 1) {
        std::cout << " X ";
      } else {
        std::cout << " . ";
      }
    }
    std::cout << "|\n";
  }
  std::cout << "  +------------------------+\n";
  std::cout << "    a  b  c  d  e  f  g  h\n";
  std::cout << "Bitboard (hex): 0x" << std::hex << bb << std::dec << "\n\n";
}

void Position::PrintAllBitboards() const {
  std::cout << "\n======================================\n";
  std::cout << "         FULL BITBOARD AUDIT          \n";
  std::cout << "======================================\n";

  print_bitboard(pawns<White>(), "White Pawns");
  print_bitboard(knights<White>(), "White Knights");
  print_bitboard(bishops<White>(), "White Bishops");
  print_bitboard(rooks<White>(), "White Rooks");
  print_bitboard(queens<White>(), "White Queen");
  print_bitboard(kings<White>(), "White King");
  print_bitboard(occupancy(White), "TOTAL White Occupancy");

  print_bitboard(pawns<Black>(), "Black Pawns");
  print_bitboard(knights<Black>(), "Black Knights");
  print_bitboard(bishops<Black>(), "Black Bishops");
  print_bitboard(rooks<Black>(), "Black Rooks");
  print_bitboard(queens<Black>(), "Black Queen");
  print_bitboard(kings<Black>(), "Black King");
  print_bitboard(occupancy(Black), "TOTAL Black Occupancy");

  print_bitboard(occupancy(), "TOTAL Board Occupancy");
}

constexpr Piece charToPiece(char c) {
  switch (c) {
  case 'P':
    return WhitePawn;
  case 'N':
    return WhiteKnight;
  case 'B':
    return WhiteBishop;
  case 'R':
    return WhiteRook;
  case 'Q':
    return WhiteQueen;
  case 'K':
    return WhiteKing;
  case 'p':
    return BlackPawn;
  case 'n':
    return BlackKnight;
  case 'b':
    return BlackBishop;
  case 'r':
    return BlackRook;
  case 'q':
    return BlackQueen;
  case 'k':
    return BlackKing;
  default:
    return None;
  }
}

constexpr char pieceToChar(Piece p) {
  switch (p) {
  case WhitePawn:
    return 'P';
  case WhiteKnight:
    return 'N';
  case WhiteBishop:
    return 'B';
  case WhiteRook:
    return 'R';
  case WhiteQueen:
    return 'Q';
  case WhiteKing:
    return 'K';
  case BlackPawn:
    return 'p';
  case BlackKnight:
    return 'n';
  case BlackBishop:
    return 'b';
  case BlackRook:
    return 'r';
  case BlackQueen:
    return 'q';
  case BlackKing:
    return 'k';
  default:
    return '.';
  }
}

Position::Position(const std::string &FEN)
    : stm(White), state(nullptr), stateCount(0), fullMoveCounter(1) {
  std::fill(std::begin(PiecesBB), std::end(PiecesBB), EMPTY_BB);
  std::fill(std::begin(board), std::end(board), None);
  occupancyWhite = occupancyBlack = occupancyAll = EMPTY_BB;

  parseFEN(FEN);
}

void Position::parseFEN(const std::string &FEN) {

  std::fill(std::begin(PiecesBB), std::end(PiecesBB), EMPTY_BB);
  std::fill(std::begin(board), std::end(board), None);
  historyCount = 0;
  mgScore = 0;
  egScore = 0;
  gamePhase = 0;

  stateCount = 1;
  state = &stateStack[0];
  state->enpassantSquare = NO_SQ;
  state->castlingRights = NO_CASTLING;
  state->halfMoveClock = 0;
  state->hashKey = 0;
  state->captured = None;
  state->previous = nullptr;

  std::stringstream ss(FEN);
  std::string part;

  // piece placement
  ss >> part;
  int rank = 7, file = 0;
  for (char ch : part) {
    if (ch == '/') {
      rank--;
      file = 0;
    } 
    else if (isdigit(ch)) {
      int skip = ch - '0';
      //prevent skipping past the H-file (file 7)
      if (file + skip <= 8) {
          file += skip;
      } else {
          file = 8; //cap it safely at the edge
      }
    } 
    else {
      Piece p = charToPiece(ch);
      if (p != None) {
        //only place the piece if we are strictly on the board
        if (file <= 7 && rank >= 0 && rank <= 7) {
          Square sq = makesquare(File(file), Rank(rank));
          placePiece(p, sq);
        }
        file++;
      }
    }
  }

  // side to move
  ss >> part;
  stm = (part == "w") ? White : Black;

  // castling
  ss >> part;
  if (part != "-") {
    for (char ch : part) {
      switch (ch) {
      case 'K':
        state->castlingRights |= WHITE_OO;
        break;
      case 'Q':
        state->castlingRights |= WHITE_OOO;
        break;
      case 'k':
        state->castlingRights |= BLACK_OO;
        break;
      case 'q':
        state->castlingRights |= BLACK_OOO;
        break;
      }
    }
  }

  // en passant
  ss >> part;
  if (part != "-") {
    state->enpassantSquare =
        makesquare(File(part[0] - 'a'), Rank(part[1] - '1'));
  }

  // halfmove clock
  int halfmove = 0;
  if ((ss >> halfmove)) {
    state->halfMoveClock = static_cast<U8>(halfmove);
  } else {
    state->halfMoveClock = 0;
  }

  // fullmove number
  int fullmove = 1;
  if (ss >> fullmove) {
    fullMoveCounter = static_cast<uint16_t>(fullmove);
  } else {
    fullMoveCounter = 1;
  }

  occupancyWhite = pawns<White>() | knights<White>() | bishops<White>() |
                   rooks<White>() | queens<White>() | kings<White>();
  occupancyBlack = pawns<Black>() | knights<Black>() | bishops<Black>() |
                   rooks<Black>() | queens<Black>() | kings<Black>();
  occupancyAll = occupancyWhite | occupancyBlack;

  state->hashKey = generateHashKey();
  state->pawnKey = generatePawnKey();
}

std::string Position::toFEN() const {
  std::stringstream fen;

  for (int rank = 7; rank >= 0; --rank) {
    int empty = 0;
    for (int file = 0; file < 8; ++file) {
      Square sq = makesquare(File(file), Rank(rank));
      Piece p = board[sq];

      if (p == None) {
        empty++;
      } else {
        if (empty > 0) {
          fen << empty;
          empty = 0;
        }
        fen << pieceToChar(p);
      }
    }
    if (empty > 0)
      fen << empty;
    if (rank > 0)
      fen << '/';
  }

  fen << " " << (stm == White ? 'w' : 'b');

  fen << " ";
  std::string castling;
  if (state->castlingRights & WHITE_OO)
    castling += 'K';
  if (state->castlingRights & WHITE_OOO)
    castling += 'Q';
  if (state->castlingRights & BLACK_OO)
    castling += 'k';
  if (state->castlingRights & BLACK_OOO)
    castling += 'q';
  fen << (castling.empty() ? "-" : castling);

  fen << " ";
  if (state->enpassantSquare == NO_SQ) {
    fen << "-";
  } else {
    int file = state->enpassantSquare % 8;
    int rank = state->enpassantSquare / 8;
    fen << char('a' + file) << char('1' + rank);
  }

  fen << " " << static_cast<int>(state->halfMoveClock);
  fen << " " << static_cast<int>(fullMoveCounter);

  return fen.str();
}

void Position::placePiece(Piece piece, Square sq) {
  Color color = piececolor(piece);
  PieceType pt = piecetype(piece);
  setbit(PiecesBB[piece], sq);
  setbit(occupancyAll, sq);
  setbit(color == White ? occupancyWhite : occupancyBlack, sq);
  board[sq] = piece;

  // Incremental eval update
  int sign = (color == White) ? 1 : -1;
  using namespace ASTROVE::eval;
  mgScore += sign * (openingScore(PSQT[pt][color][sq]) +
                     openingScore(PieceValues[pt]));
  egScore += sign * (endgameScore(PSQT[pt][color][sq]) +
                     endgameScore(PieceValues[pt]));
  gamePhase += PiecePhaseValue[piece];
}

void Position::removePiece(Square sq) {
  Piece piece = board[sq];
  if (piece != None) {
    Color color = piececolor(piece);
    PieceType pt = piecetype(piece);
    clearbit(PiecesBB[piece], sq);
    clearbit(occupancyAll, sq);
    clearbit(color == White ? occupancyWhite : occupancyBlack, sq);
    board[sq] = None;

    // Incremental eval update
    int sign = (color == White) ? 1 : -1;
    using namespace ASTROVE::eval;
    mgScore -= sign * (openingScore(PSQT[pt][color][sq]) +
                       openingScore(PieceValues[pt]));
    egScore -= sign * (endgameScore(PSQT[pt][color][sq]) +
                       endgameScore(PieceValues[pt]));
    gamePhase -= PiecePhaseValue[piece];
  }
}

void Position::movePiece(Square from, Square to) {
  Piece piece = board[from];
  removePiece(from);
  placePiece(piece, to);
}

uint64_t Position::generatePawnKey() const {
  uint64_t hash = 0ULL;
  for (int piece : {WhitePawn, BlackPawn}) {
    Bitboard bb = PiecesBB[piece];
    while (bb) {
      Square sq = poplsb(bb);
      hash ^= zobrist.pieceKeys[piece][sq];
    }
  }
  return hash;
}

uint64_t Position::generateHashKey() const {
  uint64_t hash = 0ULL;

  for (int piece = 0; piece < 12; ++piece) {
    Bitboard bb = PiecesBB[piece];
    while (bb) {
      Square sq = poplsb(bb);
      hash ^= zobrist.pieceKeys[piece][sq];
    }
  }

  if (state->enpassantSquare != NO_SQ) {
    hash ^= zobrist.enpassantKeys[state->enpassantSquare & 7];
  }

  hash ^= zobrist.castlingKeys[state->castlingRights];

  if (stm == Black) {
    hash ^= zobrist.sideKey;
  }

  return hash;
}

void Position::print() {
  std::cout << "\n  +------------------------+\n";
  for (int rank = 7; rank >= 0; --rank) {
    std::cout << rank + 1 << " |";
    for (int file = 0; file < 8; ++file) {
      Square sq = makesquare(File(file), Rank(rank));
      char ch = pieceToChar(board[sq]);
      std::cout << " " << ch << " ";
    }
    std::cout << "|\n";
  }
  std::cout << "  +------------------------+\n";
  std::cout << "    a  b  c  d  e  f  g  h\n\n";
  std::cout << "Hash: 0x" << std::hex << state->hashKey << std::dec << "\n";
  std::cout << "Side: " << (stm == White ? "White" : "Black") << "\n\n";
}
