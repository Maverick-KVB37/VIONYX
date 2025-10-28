#pragma once

#include "../board/position.h"
#include "../board/movegen.h"
#include "../table/tt.h"
#include "../ordering/ordering.h"
#include "../evaluation/evaluation.h"
#include "timemanager.h"

#include <atomic>
#include <chrono>
#include <vector>
#include <cstdint>
#include <algorithm>

using ASTROVE::eval::Evaluator;

// ==================== HOW THIS WORKS ===========================
// When We call think() the searcher perform the following steps
//
// 1. Initializes internal state, search limits, and timers.
// 2. Runs iterative deepening — calling pvs() (Principal Variation Search)
//    at each increasing depth.
// 3. Uses quiescence() at leaf nodes for tactical stability (avoiding
//    horizon effects).
// 4. Orders moves efficiently using the MoveOrderer for better pruning.
// 5. Uses the Transposition Table (TT) to skip already-explored positions.
// 6. Checks stop/time flags periodically to stay responsive under UCI control.
// 7. Sends UCI “info” updates (depth, score, PV) for GUI feedback.
// 8. Returns the best move found after the time or depth limit is reached.
//
// ============================================================================

namespace Search {

// constants
constexpr int MAX_PLY = 256;
constexpr int INFINITE = 50000;
constexpr int MATE_SCORE = 49000;
constexpr int TB_WIN_SCORE = 48000;
constexpr int MATE_BOUND = MATE_SCORE - MAX_PLY;
constexpr int NO_RAZOR=-9999999;
// Forward declarations
struct SearchLimits;
struct PVLine;
struct SearchStack;
struct SearchInfo;

// stores best play
struct PVLine {
    Move moves[MAX_PLY];
    int length = 0;

    void clear() { length = 0; }

    void update(Move move, const PVLine& childPV) {
        moves[0] = move;
        length = 1;
        
        // Copy the child's PV into this one
        int maxCopy = std::min(childPV.length, MAX_PLY - 1);
        for (int i = 0; i < maxCopy; ++i) {
                moves[length] = childPV.moves[i];
                length++;
        }
    }
};

// store data for a ply 
struct SearchStack {
    Move currentMove = NO_MOVE;
    Move excludedMove = NO_MOVE;
    Move killers[2] = {NO_MOVE, NO_MOVE};
    int staticEval = 0;
    int moveCount = 0;
    bool inCheck = false;
    int doubleExtensions = 0;
    PVLine pv;

    SearchStack() {
        clear();
    }

    void clear() {
        currentMove = excludedMove = NO_MOVE;
        killers[0] = NO_MOVE;
        killers[1] = NO_MOVE;
        staticEval = moveCount = doubleExtensions = 0;
        inCheck = false;
        pv.clear();
    }
};

// tells the limits for the current search
struct SearchLimits {
    int depth = MAX_PLY;
    uint64_t nodes = UINT64_MAX;
    int movetime = 0;
    int movestogo = 0;
    int time[2] = {0, 0};
    int inc[2] = {0, 0};
    bool infinite = false;
    bool ponder = false;
    std::vector<Move> searchmoves;
};

// collect data for uci output
struct SearchInfo {
    int depth = 0;
    int seldepth = 0;
    uint64_t nodes = 0;
    uint64_t tbhits = 0;
    int score = 0;
    int time = 0;
    uint64_t nps = 0;
    PVLine pv;
    bool isMate = false;
    int mateIn = 0;
    int hashfull = 0;

    void clear() {
        depth = seldepth = score = time = mateIn = hashfull = 0;
        nodes = tbhits = nps = 0;
        isMate = false;
        pv.clear();
    }
};

// ==================== SEARCH CLASS ====================
/*------------------------------------------
| so this find the best move by exploring game tree and also this
| this manages all move generation evaluation time control move ordering communication with GUI
*/

class Searcher {
public:
    // constructor
    explicit Searcher(Position& pos, TranspositionTable& tt);

    // start search with given limits and return best move found
    Move think(const SearchLimits& limits);

    void stop() { stopFlag = true; }
    //reset the internal state for a new game
    void newGame();

private:
    // --- search algorithm ---
    //negamax with alpha-beta pruning and PV node
    template <Color Us, bool PvNode>
    int pvs(int depth, int ply, int alpha, int beta, bool cutNode);

    //extend search at leaf of tree for avoiding horizon effect
    template <Color Us>
    int quiescence(int alpha, int beta, int ply);

    // main search loop
    void iterative_deepening();

    //checks search should be stop due to time constrint
    void check_time();

    //update uci info for uci output
    void update_uci_info(int depth, int score, const PVLine& pv);

    //functions for draw(for repetition,50-move rule)
    bool is_draw(int ply) const;
    //converts score for transposition table
    int score_to_tt(int score, int ply) const;
    int score_from_tt(int score, int ply) const;

    // Null Move Pruning
    template <Color c>
    bool tryNullMove(int alpha,int beta,int depth,int ply,int& score);

    template <Color c>
    bool tryReverseFutility(int beta, int depth, int ply, int& score);

    template <Color c>
    bool shouldPruneMove(int depth,int moveCount,bool inCheck,
                                     bool isCapture,bool isPromotion,bool givesCheck);
    
    template <Color c>
    int tryRazoring(int alpha, int depth, int ply);

    template <Color c>
    bool canFutilityPrune(int alpha,int depth,int staticEval,
                                      bool isCapture,bool isPromotion,bool givesCheck);
    
    void initLMR();
    int getLMRReduction(int depth, int moveCount, bool isPV, bool improving);
    bool shouldReduceMove(int depth, int legalMoves, bool inCheck,
                          bool isCapture, bool isPromotion);
    // --- DATA MEMBERS ---
    Position& pos;
    TranspositionTable& tt;

    Evaluator eval;
    MoveGenerator gen;
    TimeManager tm;
    MoveOrderer orderer;

    // Search state
    std::atomic<bool> stopFlag;
    SearchLimits limits; //current search constraint
    SearchInfo info; //current search statistics
    std::chrono::steady_clock::time_point startTime;
    uint64_t nodes;
    int selDepth; 

    // per ply data for deep search
    SearchStack stack[MAX_PLY + 10];
};

} // namespace Search
