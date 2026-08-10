#pragma once

#include "../board/movegen.h"
#include "../board/position.h"
#include "../core/zobrist.h"
#include "../search/search.h"
#include "../table/tt.h"
#include <sstream>
#include <thread>

class UCI {
public:
  Position *pos;
  TranspositionTable tt;
  Search::Searcher *searcher;
  MoveGenerator gen;
  std::istringstream iss;
  std::thread searchThread;

  UCI();
  ~UCI();
  void uciLoop();
  void bootEngine();
  Move parseMove(const std::string &moveUci);
};
