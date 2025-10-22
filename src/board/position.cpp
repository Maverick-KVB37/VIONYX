#include "position.h"
#include "../core/zobrist.h"
#include "../core/types.h"
#include "../core/bitboard.h"
#include <iostream>
#include <sstream>

// A helper function to print a bitboard for debugging
void print_bitboard(Bitboard bb, const std::string& name) {
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

void Position::print_all_bitboards() const {
    std::cout << "\n======================================\n";
    std::cout << "         FULL BITBOARD AUDIT          \n";
    std::cout << "======================================\n";

    print_bitboard(pawns<White>(),   "White Pawns");
    print_bitboard(knights<White>(), "White Knights");
    print_bitboard(bishops<White>(), "White Bishops");
    print_bitboard(rooks<White>(),   "White Rooks");
    print_bitboard(queens<White>(),  "White Queen");
    print_bitboard(kings<White>(),   "White King");
    print_bitboard(occupancy(White), "TOTAL White Occupancy");

    print_bitboard(pawns<Black>(),   "Black Pawns");
    print_bitboard(knights<Black>(), "Black Knights");
    print_bitboard(bishops<Black>(), "Black Bishops");
    print_bitboard(rooks<Black>(),   "Black Rooks");
    print_bitboard(queens<Black>(),  "Black Queen");
    print_bitboard(kings<Black>(),   "Black King");
    print_bitboard(occupancy(Black), "TOTAL Black Occupancy");
    
    print_bitboard(occupancy(),      "TOTAL Board Occupancy");
}

// Getter function for the char-to-piece map
const std::map<char, Piece>& getCharToPiece() {
    static const std::map<char, Piece> charToPieceMap = {
        {'P', WhitePawn},   {'N', WhiteKnight}, {'B', WhiteBishop}, {'R', WhiteRook}, {'Q', WhiteQueen}, {'K', WhiteKing},
        {'p', BlackPawn},   {'n', BlackKnight}, {'b', BlackBishop}, {'r', BlackRook}, {'q', BlackQueen}, {'k', BlackKing}
    };
    return charToPieceMap;
}

// Getter function for the piece-to-char map
const std::map<Piece, char>& getPieceToChar() {
    static const std::map<Piece, char> pieceToCharMap = {
        {WhitePawn, 'P'},   {WhiteKnight, 'N'}, {WhiteBishop, 'B'}, {WhiteRook, 'R'}, {WhiteQueen, 'Q'}, {WhiteKing, 'K'},
        {BlackPawn, 'p'},   {BlackKnight, 'n'}, {BlackBishop, 'b'}, {BlackRook, 'r'}, {BlackQueen, 'q'}, {BlackKing, 'k'}
    };
    return pieceToCharMap;
}

Position::Position(const std::string& FEN) 
    : stm(White), state(nullptr), stateCount(0), fullMoveCounter(1) {
    std::fill(std::begin(PiecesBB), std::end(PiecesBB), EMPTY_BB);
    std::fill(std::begin(board), std::end(board), None);
    occupancyWhite = occupancyBlack = occupancyAll = EMPTY_BB;
    
    parseFEN(FEN);
}

void Position::parseFEN(const std::string& FEN) {
    const auto& charToPiece = getCharToPiece();

    // Reset board
    std::fill(std::begin(PiecesBB), std::end(PiecesBB), EMPTY_BB);
    std::fill(std::begin(board), std::end(board), None);
    
    // Initialize first state
    stateCount = 0;
    state = &stateStack[stateCount++];
    state->enpassantSquare = NO_SQ;
    state->castlingRights = NO_CASTLING;
    state->halfMoveClock = 0;
    state->hashKey = 0;
    state->captured = None;
    state->previous = nullptr;
    
    std::stringstream ss(FEN);
    std::string part;
    
    // 1. Piece placement
    ss >> part;
    int rank = 7, file = 0;
    for (char ch : part) {
        if (ch == '/') {
            rank--;
            file = 0;
        } else if (isdigit(ch)) {
            file += ch - '0';
        } else if (charToPiece.count(ch)) {
            Square sq = makesquare(File(file), Rank(rank));
            placePiece(charToPiece.at(ch), sq);
            file++;
        }
    }
    
    // 2. Side to move
    ss >> part;
    stm = (part == "w") ? White : Black;
    
    // 3. Castling
    ss >> part;
    if (part != "-") {
        for (char ch : part) {
            switch (ch) {
                case 'K': state->castlingRights |= WHITE_OO;  break;
                case 'Q': state->castlingRights |= WHITE_OOO; break;
                case 'k': state->castlingRights |= BLACK_OO;  break;
                case 'q': state->castlingRights |= BLACK_OOO; break;
            }
        }
    }
    
    // 4. En passant
    ss >> part;
    if (part != "-") {
        state->enpassantSquare = makesquare(File(part[0] - 'a'), Rank(part[1] - '1'));
    }
    
    // 5. Halfmove clock
    int halfmove=0;
    if ((ss >>halfmove)) {
        state->halfMoveClock = static_cast<U8>(halfmove);
    }
    else{
        state->halfMoveClock=0;
    }
    
    // 6. Fullmove number
    int fullmove=1;
    if (ss>>fullmove) {
        fullMoveCounter=static_cast<uint16_t>(fullmove);
    } else {
        fullMoveCounter=1;
    }
    
    // Update occupancy
    occupancyWhite = pawns<White>() | knights<White>() | bishops<White>() | 
                     rooks<White>() | queens<White>() | kings<White>();
    occupancyBlack = pawns<Black>() | knights<Black>() | bishops<Black>() | 
                     rooks<Black>() | queens<Black>() | kings<Black>();
    occupancyAll = occupancyWhite | occupancyBlack;
    
    // Generate hash
    state->hashKey = generateHashKey();
}

std::string Position::toFEN() const {
    const auto& pieceToChar = getPieceToChar();
    std::stringstream fen;
    
    // 1. Piece placement
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
                fen << pieceToChar.at(p);
            }
        }
        if (empty > 0) fen << empty;
        if (rank > 0) fen << '/';
    }
    
    // 2. Side to move
    fen << " " << (stm == White ? 'w' : 'b');
    
    // 3. Castling rights
    fen << " ";
    std::string castling;
    if (state->castlingRights & WHITE_OO)  castling += 'K';
    if (state->castlingRights & WHITE_OOO) castling += 'Q';
    if (state->castlingRights & BLACK_OO)  castling += 'k';
    if (state->castlingRights & BLACK_OOO) castling += 'q';
    fen << (castling.empty() ? "-" : castling);
    
    // 4. En passant
    fen << " ";
    if (state->enpassantSquare == NO_SQ) {
        fen << "-";
    } else {
        int file=state->enpassantSquare%8;
        int rank=state->enpassantSquare/8;
        fen<<char('a'+file)<<char('1'+rank);
    }

    // 5. Halfmove clock
    fen << " " << static_cast<int>(state->halfMoveClock);
    
    // 6. Fullmove number
    fen << " " << static_cast<int>(fullMoveCounter);
    
    return fen.str();
}

void Position::placePiece(Piece piece, Square sq) {
    Color color=piececolor(piece);
    setbit(PiecesBB[piece], sq);
    setbit(occupancyAll,sq);
    setbit(color==White ? occupancyWhite : occupancyBlack,sq);
    board[sq] = piece;
}

void Position::removePiece(Square sq) {
    Piece piece = board[sq];
    if (piece != None) {
        Color color = piececolor(piece);
        clearbit(PiecesBB[piece], sq);
        clearbit(occupancyAll, sq);
        clearbit(color == White ? occupancyWhite : occupancyBlack, sq);
        board[sq] = None;
    }
}

void Position::movePiece(Square from, Square to) {
    Piece piece = board[from];
    removePiece(from);
    placePiece(piece,to);
}

uint64_t Position::generateHashKey() const {
    uint64_t hash = 0ULL;
    
    // Hash all pieces
    for (int piece = 0; piece < 12; ++piece) {
        Bitboard bb = PiecesBB[piece];
        while (bb) {
            Square sq = poplsb(bb);
            hash ^= zobrist.pieceKeys[piece][sq];
        }
    }
    
    // Hash en passant (ONLY file matters!)
    if (state->enpassantSquare != NO_SQ) {
        hash ^= zobrist.enpassantKeys[state->enpassantSquare & 7];
    }
    
    // Hash castling
    hash ^= zobrist.castlingKeys[state->castlingRights];
    
    // Hash side to move
    if (stm == Black) {
        hash ^= zobrist.sideKey;
    }
    
    return hash;
}

void Position::print() {
    const auto& pieceToChar = getPieceToChar();
    std::cout << "\n  +------------------------+\n";
    for (int rank = 7; rank >= 0; --rank) {
        std::cout << rank + 1 << " |";
        for (int file = 0; file < 8; ++file) {
            Square sq = makesquare(File(file), Rank(rank));
            char ch = '.';
            if (pieceToChar.count(board[sq])) {
                ch = pieceToChar.at(board[sq]);
            }
            std::cout << " " << ch << " ";
        }
        std::cout << "|\n";
    }
    std::cout << "  +------------------------+\n";
    std::cout << "    a  b  c  d  e  f  g  h\n\n";
    //std::cout << "FEN: " << toFEN() << "\n";
    std::cout << "Hash: 0x" << std::hex << state->hashKey << std::dec << "\n";
    std::cout << "Side: " << (stm == White ? "White" : "Black") << "\n\n";
}

