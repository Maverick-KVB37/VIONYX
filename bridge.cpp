#include <emscripten/emscripten.h>
#include <string>
#include <sstream>
#include <iostream>

// Engine headers
#include "src/board/position.h"
#include "src/board/movegen.h"
#include "src/search/search.h"
#include "src/table/tt.h"
#include "src/core/zobrist.h"
#include "src/core/magic.h"
#include "src/evaluation/evaluation.h"
#include "src/evaluation/psqt.h"

// Global engine state (persists between calls)
static Position* g_pos = nullptr;
static TranspositionTable* g_tt = nullptr;
static Search::Searcher* g_searcher = nullptr;
static MoveGenerator g_gen;
static bool g_initialized = false;

extern "C" {

EMSCRIPTEN_KEEPALIVE
void init_engine() {
    if (g_initialized) return;

    // Initialize magic bitboards
    Astrove::magic::init();

    // Initialize Zobrist hashing
    zobrist.init();

    // Initialize evaluation tables
    ASTROVE::eval::InitializePieceSquareTable();

    // Create TT (16 MB for WASM — browser-friendly)
    g_tt = new TranspositionTable();
    g_tt->init(16);

    // Create position and searcher
    g_pos = new Position();
    g_searcher = new Search::Searcher(*g_pos, *g_tt);

    g_initialized = true;
}

EMSCRIPTEN_KEEPALIVE
const char* get_best_move(const char* fen_str, int depth) {
    static std::string result;

    if (!g_initialized) {
        result = "0000";
        return result.c_str();
    }

    // Clamp depth to reasonable range for browser
    if (depth < 1) depth = 1;
    if (depth > 12) depth = 12;

    // Set up position from FEN
    g_pos->parseFEN(std::string(fen_str));

    // Clear searcher state for fresh search
    g_searcher->newGame();
    g_tt->newSearch();

    // Set up search limits
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

    // Suppress stdout output from the search (bestmove, info lines)
    std::streambuf* old_cout = std::cout.rdbuf();
    std::ostringstream devnull;
    std::cout.rdbuf(devnull.rdbuf());

    // Run search
    Move bestMove = g_searcher->think(limits);

    // Restore stdout
    std::cout.rdbuf(old_cout);

    // Convert move to UCI string
    if (bestMove.from() == bestMove.to() || bestMove.from() >= 64 || bestMove.to() >= 64) {
        result = "0000";
    } else {
        result = bestMove.to_uci_string();
    }

    return result.c_str();
}

} // extern "C"
