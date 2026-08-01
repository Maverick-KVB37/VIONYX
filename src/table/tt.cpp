#include "tt.h"
#include <algorithm>
#include <cstdlib>
#include <cstring>

TranspositionTable::TranspositionTable()
    : table(nullptr), numEntries(0), currentAge(0) {}

TranspositionTable::~TranspositionTable() {
  if (table) {
    ::operator delete(table);
  }
}

/*
| Clear Transposition Table                                                   |
| Resets all entries in the transposition table to their default state.       |
*/
void TranspositionTable::clear() {
  if (!table)
    return;
  std::fill(table, table + numEntries, TTEntry{});
}

/*
| Initialize Transposition Table                                              |
| Allocates the specified amount of memory (in MB) for the table and          |
| computes the maximum number of entries it can hold.                         |
*/
void TranspositionTable::init(size_t sizeMB) {
  if (sizeMB < 1)
    sizeMB = 1;

  size_t bytes = sizeMB * 1024ULL * 1024ULL;
  numEntries = bytes / sizeof(TTEntry);
  if (numEntries == 0) {
    std::cerr << "Failed TT allocation.\n";
    throw std::bad_alloc();
  }

  try {
    table =
        static_cast<TTEntry *>(::operator new(numEntries * sizeof(TTEntry)));
  } catch (const std::bad_alloc &) {
    std::cerr << "FATAL: TT allocation failed for " << sizeMB << " MB.\n";
    throw;
  }

  clear();
  std::cout << "info string TT initialized: " << sizeMB << " MB (" << numEntries
            << " entries)\n";
}

/*
| New Search (Aging)                                                          |
| Increments the current age of the table. This is used in the replacement    |
| scheme to overwrite older, less relevant entries.                           |
*/
void TranspositionTable::newSearch() { currentAge = (currentAge + 1) & 0xFF; }

/*
| Store Entry                                                                 |
| Saves the evaluation details and best move for a specific position into the |
| transposition table. Uses an aging and depth-based replacement scheme.      |
*/
void TranspositionTable::store(uint64_t key, int depth, int flag, int score,
                               int eval, int ply, Move bestMove) {

  size_t index = key % (numEntries / MAX_BUCKETS);
  TTEntry *bucket = &table[index * MAX_BUCKETS];

  // normalize mate score for storage
  if (score >= 49000)
    score += ply;
  else if (score <= -49000)
    score -= ply;

  /*
  | Replacement Scheme | | Prefers to replace entries that are older or searched
  to a shallower depth. |
  */
  TTEntry *replace = &bucket[0];
  for (int i = 0; i < MAX_BUCKETS; ++i) {
    if (bucket[i].key == key) {
      replace = &bucket[i];
      break;
    }
    if (bucket[i].depth < replace->depth || bucket[i].age != currentAge)
      replace = &bucket[i];
  }

  replace->key = key;
  replace->depth = static_cast<int8_t>(depth);
  replace->flag = static_cast<uint8_t>(flag);
  replace->score = static_cast<int16_t>(score);
  replace->eval = static_cast<int16_t>(eval);
  replace->age = static_cast<uint8_t>(currentAge);
  replace->bestMove = bestMove;
}

/*
| Probe Transposition Table                                                   |
| Looks up the given position in the table. If found, retrieves the best move |
| and can cause an immediate cutoff if the stored score is reliable.          |
*/
bool TranspositionTable::probe(uint64_t key, int depth, int alpha, int beta,
                               int &score, Move &bestMove, int ply) const {

  size_t index = key % (numEntries / MAX_BUCKETS);
  TTEntry *bucket = &table[index * MAX_BUCKETS];

  for (int i = 0; i < MAX_BUCKETS; ++i) {
    const TTEntry &entry = bucket[i];
    if (entry.key != key)
      continue;

    bestMove = entry.bestMove;
    if (entry.depth < depth)
      return false;

    int stored = entry.score;
    if (stored >= 49000)
      stored -= ply;
    else if (stored <= -49000)
      stored += ply;

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

/*
| Probe for Singular Extension                                                |
| Retrieves table information without cutoff logic to help determine if       |
| a move is singular and should be extended.                                  |
*/
bool TranspositionTable::probeForSE(uint64_t key, int &ttDepth, int &ttFlag,
                                    int &score, Move &bestMove, int ply) const {
  size_t index = key % (numEntries / MAX_BUCKETS);
  TTEntry *bucket = &table[index * MAX_BUCKETS];

  for (int i = 0; i < MAX_BUCKETS; ++i) {
    const TTEntry &entry = bucket[i];
    if (entry.key != key)
      continue;

    bestMove = entry.bestMove;
    ttDepth = entry.depth;
    ttFlag = entry.flag;

    int stored = entry.score;
    if (stored >= 49000)
      stored -= ply;
    else if (stored <= -49000)
      stored += ply;

    score = stored;
    return true;
  }
  return false;
}

/*
| Hashfull Approximation                                                      |
| Samples the first 1000 entries to estimate how full the transposition table |
| is (per mill, i.e., out of 1000). Used for UCI output.                      |
*/
int TranspositionTable::hashfull() const {
  if (!table || numEntries == 0)
    return 0;
  int cnt = 0;

  int sampled = std::min(1000, static_cast<int>(numEntries));

  for (int i = 0; i < sampled; ++i) {
    if (table[i].key != 0)
      cnt++;
  }

  return (cnt * 1000) / sampled;
}
