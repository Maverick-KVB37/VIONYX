#include <emscripten/emscripten.h>
#include <string>
#include <sstream>
#include <iostream>

#include "src/board/position.h"
#include "src/board/movegen.h"
#include "src/search/search.h"
#include "src/table/tt.h"
#include "src/core/zobrist.h"
#include "src/core/magic.h"
#include "src/evaluation/evaluation.h"
#include "src/evaluation/psqt.h"

//global engine state
static Position* Gpos = nullptr;
static TranspositionTable* Gtt = nullptr;
static Search::Searcher* Gsearcher = nullptr;
static MoveGenerator Ggen;
static bool Ginitialized = false;

extern "C" {

EMSCRIPTEN_KEEPALIVE
void InitEngine() {
    if (Ginitialized) return;

    Astrove::magic::init();
    zobrist.init();
    ASTROVE::eval::InitializePieceSquareTable();

    Gtt = new TranspositionTable();
    Gtt->init(16); // 16 MB for WASM

    Gpos = new Position();
    Gsearcher = new Search::Searcher(*Gpos, *Gtt);

    Ginitialized = true;
}

EMSCRIPTEN_KEEPALIVE
const char* GetBestMove(const char* fenstr, int depth) {
    static std::string result;

    if (!Ginitialized) {
        result = "0000";
        return result.c_str();
    }

    if (depth < 1) depth = 1;
    if (depth > 12) depth = 12;

    Gpos->parseFEN(std::string(fenstr));

    Gsearcher->newGame();
    Gtt->newSearch();

    Search::SearchLimits limits;
    limits.depth = depth;
    limits.nodes = UINT64_MAX;
    limits.movetime = 0;
    limits.movestogo = 0;
    limits.wtime = 0;
    limits.btime = 0;
    limits.winc = 0;
    limits.binc = 0;
    limits.infinite = false;
    limits.ponder = false;

    //suppress stdout during search
    std::streambuf* oldcout = std::cout.rdbuf();
    std::ostringstream devnull;
    std::cout.rdbuf(devnull.rdbuf());

    Move bestMove = Gsearcher->think(limits);

    std::cout.rdbuf(oldcout);

    if (bestMove.from() == bestMove.to() || bestMove.from() >= 64 || bestMove.to() >= 64) {
        result = "0000";
    } else {
        result = bestMove.ToUciString();
    }

    return result.c_str();
}

} // extern "C"
