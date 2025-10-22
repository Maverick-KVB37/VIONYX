#pragma once
#include "../core/move.h"
#include <cstdint>
#include <cstdlib>
#include <iostream>


// Transposition Table constants
constexpr int HASH_FLAG_EXACT = 0;
constexpr int HASH_FLAG_ALPHA = 1;
constexpr int HASH_FLAG_BETA  = 2;
constexpr int NO_HASH_ENTRY   = 32002;

constexpr int MAX_BUCKETS = 2; // number of tt entrie per index


struct alignas(64) TTEntry {
    uint64_t key;       // zobrist key of the position
    int16_t score;      // stored search score
    int16_t eval;       // static evaluation
    uint8_t depth;      // search depth of entry
    uint8_t flag;       // alpha / beta / exact
    uint8_t age;        // for replacement policy
    uint8_t padding;    // alignment to 8 bytes
    Move bestMove;      // best move found
};

// ----------------------------------------------------------
// Transposition Table Class
// ----------------------------------------------------------
class TranspositionTable {
public:
    TranspositionTable();
    ~TranspositionTable();

    void init(size_t sizeMB);
    void clear();
    void newSearch();                            // increments age

    void store(uint64_t key, int depth, int flag,
               int score, int eval, int ply, Move bestMove);

    bool probe(uint64_t key, int depth, int alpha,
               int beta, int& score, Move& bestMove, int ply) const;

    int hashfull() const;                        // occupancy (for UCI display)

private:
    TTEntry* table = nullptr;
    size_t numEntries = 0;
    uint8_t currentAge = 0;
};
