#include <sstream>
#include <atomic>
#include <thread>
#include <mutex>
#include <algorithm>
#include <chrono>
#include <iostream>
#include <cstring>
#include "uci.h"
#include "../core/zobrist.h"
#include "../core/magic.h"
#include "../evaluation/evaluation.h"

// Don't redefine defaultFEN - it's already in position.h

UCI::UCI() : pos(nullptr), tt() {
    pos = new Position("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
    searcher = new Search::Searcher(*pos, tt);
}

UCI::~UCI() {
    delete searcher;
    delete pos;
}

void UCI::uciLoop() {
    bootEngine();

    std::string line;
    while (std::getline(std::cin, line)) {
        iss.clear();
        iss.str(line);

        std::string command;
        iss >> command;

        if (command == "uci") {
            std::cout << "id name Astrove\n";
            std::cout << "id author Kirti Vardhan Bhushan\n";
            std::cout << "uciok\n";
        }
        else if (command == "isready") {
            std::cout << "readyok\n";
        }
        else if (command == "ucinewgame") {
            tt.clear();

            if (searcher) {
                searcher->newGame();
            }
        }
        else if (command == "position") {
            std::string token;
            iss >> token;

            if (token == "startpos") {
                pos->parseFEN(defaultFEN);

                // Check if there are moves to apply
                if (iss >> token && token == "moves") {
                    std::string moveUci;
                    while (iss >> moveUci) {
                        Move move = parseMove(moveUci);
                        if (pos->sideToMove() == White) {
                            pos->makemove<White>(move);
                        }
                        else {
                            pos->makemove<Black>(move);
                        }
                    }
                }
            }
            else if (token == "fen") {
                std::string fen, part;
                int fenParts = 0;
                // Read 6 FEN parts
                while (fenParts < 6 && iss >> part) {
                    if (part == "moves") break;
                    fen += part + " ";
                    fenParts++;
                }
                if (!fen.empty() && fen.back() == ' ') fen.pop_back();

                pos->parseFEN(fen);

                // Apply moves if present
                if (part == "moves" || (iss >> part && part == "moves")) {
                    std::string moveUci;
                    while (iss >> moveUci) {
                        Move move = parseMove(moveUci);
                        if (pos->sideToMove() == White) {
                            pos->makemove<White>(move);
                        }
                        else {
                            pos->makemove<Black>(move);
                        }
                    }
                }
            }

            std::cout << "info string Position set\n";
        }
        else if (command == "go") {
            // Prepare SearchLimits structure
            Search::SearchLimits limits;
            limits.depth = Search::MAX_PLY;
            limits.nodes = UINT64_MAX;
            limits.movetime = 0;
            limits.movestogo = 0;
            limits.wtime = 0;
            limits.btime = 0;  
            limits.winc = 0;   
            limits.binc = 0;
            limits.infinite = false;
            limits.ponder = false;

            std::string token;
            while (iss >> token) {
                if (token == "movetime") {
                    iss >> limits.movetime;
                }
                else if (token == "wtime") {
                    iss >> limits.wtime;
                }
                else if (token == "btime") {
                    iss >> limits.btime;
                }
                else if (token == "winc") {
                    iss >> limits.winc;
                }
                else if (token == "binc") {
                    iss >> limits.binc;
                }
                else if (token == "movestogo") {
                    iss >> limits.movestogo;
                }
                else if (token == "depth") {
                    iss >> limits.depth;
                }
                else if (token == "infinite") {
                    limits.infinite = true;
                }
                else if (token == "nodes") {
                    iss >> limits.nodes;
                }
            }

            // Call think() which handles search
            searcher->think(limits);

        }
        else if (command == "stop") {
            searcher->stop();
        }
        else if (command == "quit") {
            searcher->stop();
            break;
        }
        else if (command == "print") {
            pos->print();
        }
    }
}

Move UCI::parseMove(const std::string& moveUci) {
    MoveList movelist;
    
    if (pos->sideToMove() == White) {
        gen.generate_all_moves<White>(*pos, movelist);
    } else {
        gen.generate_all_moves<Black>(*pos, movelist);
    }

    if (moveUci.size() < 4) {
        return Move();
    }

    Square source = Square((moveUci[1] - '1') * 8 + (moveUci[0] - 'a'));
    Square target = Square((moveUci[3] - '1') * 8 + (moveUci[2] - 'a'));

    PieceType promoted = Nonetype;
    if (moveUci.size() == 5) {
        switch (moveUci[4]) {
            case 'q': promoted = Queen; break;
            case 'r': promoted = Rook; break;
            case 'b': promoted = Bishop; break;
            case 'n': promoted = Knight; break;
            default: promoted = Nonetype; break;
        }
    }

    for (const auto& move : movelist) {
        if (move.from() == source && move.to() == target) {
            if (promoted == Nonetype && !move.is_promotion()) {
                return move;
            }
            else if (move.is_promotion() && move.promoted_piece_type() == promoted) {
                return move;
            }
        }
    }
    return Move();
}

void UCI::bootEngine() {
    // Initialize magic bitboards
    Astrove::magic::init();
    
    // Initialize Zobrist hashing
    zobrist.init();
    
    // Initialize evaluation tables
    ASTROVE::eval::InitializePieceSquareTable();
    
    // Initialize TT
    tt.init(64); // 64 MB
    
    std::cout << "Astrove UCI-compatible engine ready\n";
}
