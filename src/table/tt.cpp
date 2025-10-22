#include "tt.h"
#include <cstring>

TranspositionTable::TranspositionTable() : table(nullptr), numEntries(0), currentAge(0) {}

TranspositionTable::~TranspositionTable() {
    if (table) {
        delete[] table;
    }
}

// clear all tt entries
void TranspositionTable::clear() {
    if (!table) return;
    for (size_t i = 0; i < numEntries; ++i) {
        table[i].key = 0;
        table[i].depth = 0;
        table[i].flag = 0;
        table[i].score = 0;
        table[i].eval = 0;
        table[i].age = 0;
        table[i].bestMove = Move();
    }
}


// allocate memory and initialize tt
void TranspositionTable::init(size_t sizeMB) {
    if (sizeMB < 1) sizeMB = 1;

    size_t bytes = sizeMB * 1024ULL * 1024ULL;
    numEntries = bytes / sizeof(TTEntry);
    if (numEntries == 0) {
        std::cerr << "Failed TT allocation.\n";
        exit(1);
    }

    if (table) free(table);
    table = static_cast<TTEntry*>(aligned_alloc(64, numEntries * sizeof(TTEntry)));
    if (!table) {
        std::cerr << "FATAL: TT allocation failed for " << sizeMB << "MB.\n";
        exit(1);
    }

    clear();
    std::cout << "info string TT initialized: " << sizeMB
              << " MB (" << numEntries << " entries)\n";
}


// begin a new search and increment age (for aging policy)
void TranspositionTable::newSearch() {
    currentAge = (currentAge + 1) & 0xFF;
}


// store position data in tt
void TranspositionTable::store(uint64_t key, int depth, int flag,
                               int score, int eval, int ply, Move bestMove) {

    size_t index = key % (numEntries / MAX_BUCKETS);
    TTEntry* bucket = &table[index * MAX_BUCKETS];

    // normalize mate score for storage
    if (score >= 49000) score += ply;
    else if (score <= -49000) score -= ply;

    // replacement scheme prefer deeper or newer entrie
    TTEntry* replace = &bucket[0];
    for (int i = 0; i < MAX_BUCKETS; ++i) {
        if (bucket[i].key == key) { replace = &bucket[i]; break; }
        if (bucket[i].depth < replace->depth || bucket[i].age != currentAge)
            replace = &bucket[i];
    }

    replace->key   = key;
    replace->depth = depth;
    replace->flag  = flag;
    replace->score = static_cast<int16_t>(score);
    replace->eval  = static_cast<int16_t>(eval);
    replace->age   = currentAge;
    replace->bestMove = bestMove;
}


// probe for a position in the tt
bool TranspositionTable::probe(uint64_t key, int depth, int alpha,
                               int beta, int& score, Move& bestMove, int ply) const {

    size_t index = key % (numEntries / MAX_BUCKETS);
    TTEntry* bucket = &table[index * MAX_BUCKETS];

    for (int i = 0; i < MAX_BUCKETS; ++i) {
        const TTEntry& entry = bucket[i];
        if (entry.key != key) continue;

        bestMove = entry.bestMove;
        if (entry.depth < depth) return false;

        int stored = entry.score;
        if (stored >= 49000) stored -= ply;
        else if (stored <= -49000) stored += ply;

        if (entry.flag == HASH_FLAG_EXACT) {
            score = stored;
            return true;
        }
        if (entry.flag == HASH_FLAG_ALPHA && stored <= alpha) {
            score = alpha;
            return true;
        }
        if (entry.flag == HASH_FLAG_BETA && stored >= beta) {
            score = beta;
            return true;
        }
    }
    return false;
}


// calculate hash table fill ratio
int TranspositionTable::hashfull() const {
    if (!table) return 0;
    int cnt = 0;
    for (int i = 0; i < 1000; ++i)
        if (table[i].key) cnt++;
    return cnt / 10; // percentage (0–100)
}
