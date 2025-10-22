#include "tt.h"
#include <cstring>
#include <cstdlib>

TranspositionTable::TranspositionTable() : table(nullptr), numEntries(0), currentAge(0) {}

TranspositionTable::~TranspositionTable() {
    if (table) {
        ::operator delete[](table, std::align_val_t(64));
    }
}

// clear all tt entries
void TranspositionTable::clear() {
    if (!table) return;
    std::memset(table, 0, numEntries * sizeof(TTEntry));
}


// allocate memory and initialize tt
void TranspositionTable::init(size_t sizeMB) {
    if (sizeMB < 1) sizeMB = 1;

    size_t bytes = sizeMB * 1024ULL * 1024ULL;
    numEntries = bytes / sizeof(TTEntry);
    if (numEntries == 0) {
        std::cerr << "Failed TT allocation.\n";
        throw std::bad_alloc();
    }

    try {
        table = static_cast<TTEntry*>(
            ::operator new[](numEntries * sizeof(TTEntry), std::align_val_t(64))
        );
    } catch (const std::bad_alloc&) {
        std::cerr << "FATAL: TT allocation failed for " << sizeMB << " MB.\n";
        throw;
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
    replace->depth = static_cast<int8_t>(depth);
    replace->flag  = static_cast<uint8_t>(flag);
    replace->score = static_cast<int16_t>(score);
    replace->eval  = static_cast<int16_t>(eval);
    replace->age   = static_cast<uint8_t>(currentAge);
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
    if (!table || numEntries==0) return 0;
    int cnt = 0;

    int sampled = std::min(1000, static_cast<int>(numEntries));
    
    for (int i = 0; i < sampled; ++i) {
        if (table[i].key != 0) cnt++;
    }
    
    return (cnt * 1000) / sampled;
}
