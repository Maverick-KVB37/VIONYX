#include "uci.h"
#include "../core/magic.h"
#include "../core/zobrist.h"
#include "../search/search.h"
#include <cstring>
#include <iostream>
#include <sstream>

UCI::UCI() : pos(nullptr), tt() {
  pos =
      new Position("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
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
      std::cout << "id name Astrove 2.0\n";
      std::cout << "id author Kirti Vardhan Bhushan\n";
      std::cout << "uciok\n";
    } else if (command == "isready") {
      std::cout << "readyok\n";
    } else if (command == "ucinewgame") {
      tt.clear();

      if (searcher) {
        searcher->newGame();
      }
    } else if (command == "position") {
      std::string token;
      iss >> token;

      if (token == "startpos") {
        if (searcher) {
          searcher->clearHistory();
        }
        pos->parseFEN(defaultFEN);

        if (iss >> token && token == "moves") {
          std::string moveUci;
          while (iss >> moveUci) {
            Move move = parseMove(moveUci);
            if (pos->sideToMove() == White) {
              pos->makemove<White>(move);
            } else {
              pos->makemove<Black>(move);
            }
          }
        }
      } else if (token == "fen") {
        std::string fen, part;
        int fenParts = 0;
        while (fenParts < 6 && iss >> part) {
          if (part == "moves")
            break;
          fen += part + " ";
          fenParts++;
        }
        if (!fen.empty() && fen.back() == ' ')
          fen.pop_back();

        pos->parseFEN(fen);

        if (part == "moves" || (iss >> part && part == "moves")) {
          std::string moveUci;
          while (iss >> moveUci) {
            Move move = parseMove(moveUci);
            if (pos->sideToMove() == White) {
              pos->makemove<White>(move);
            } else {
              pos->makemove<Black>(move);
            }
          }
        }
      }

      std::cout << "info string Position set\n";
    } else if (command == "go") {
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
        } else if (token == "wtime") {
          iss >> limits.wtime;
        } else if (token == "btime") {
          iss >> limits.btime;
        } else if (token == "winc") {
          iss >> limits.winc;
        } else if (token == "binc") {
          iss >> limits.binc;
        } else if (token == "movestogo") {
          iss >> limits.movestogo;
        } else if (token == "depth") {
          iss >> limits.depth;
        } else if (token == "infinite") {
          limits.infinite = true;
        } else if (token == "nodes") {
          iss >> limits.nodes;
        }
      }

      searcher->think(limits);

    } else if (command == "stop") {
      searcher->stop();
    } else if (command == "quit") {
      searcher->stop();
      break;
    } else if (command == "print") {
      pos->print();
    }
  }
}

Move UCI::parseMove(const std::string &moveUci) {
  MoveList movelist;

  if (pos->sideToMove() == White) {
    gen.GenerateAllMoves<White>(*pos, movelist);
  } else {
    gen.GenerateAllMoves<Black>(*pos, movelist);
  }

  if (moveUci.size() < 4) {
    return Move();
  }

  Square source = Square((moveUci[1] - '1') * 8 + (moveUci[0] - 'a'));
  Square target = Square((moveUci[3] - '1') * 8 + (moveUci[2] - 'a'));

  PieceType promoted = Nonetype;
  if (moveUci.size() == 5) {
    switch (moveUci[4]) {
    case 'q':
      promoted = Queen;
      break;
    case 'r':
      promoted = Rook;
      break;
    case 'b':
      promoted = Bishop;
      break;
    case 'n':
      promoted = Knight;
      break;
    default:
      promoted = Nonetype;
      break;
    }
  }

  for (const auto &move : movelist) {
    if (move.from() == source && move.to() == target) {
      if (promoted == Nonetype && !move.IsPromotion()) {
        return move;
      } else if (move.IsPromotion() && move.PromotedPieceType() == promoted) {
        return move;
      }
    }
  }
  return Move();
}

void UCI::bootEngine() {
  Astrove::magic::init();
  zobrist.init();
  ASTROVE::eval::InitializePieceSquareTable();
  tt.init(64);

  std::cout << "Astrove UCI-compatible engine ready\n";
}
