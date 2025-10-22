#pragma once

#include <thread>
#include <sstream>
#include "../board/position.h"
#include "../table/tt.h"
#include "../search/search.h"
#include "../board/movegen.h"
#include "../core/zobrist.h"

extern Zobrist zobristInstance;

class UCI {
public:
    Position pos;
    TranspositionTable tt;
    Search::Searcher* searcher;
    MoveGenerator gen;
    std::istringstream iss;
    std::thread searchThread;

    UCI();

    void uciLoop();
    void bootEngine();
    Move parseMove(const std::string& moveUci);
};
