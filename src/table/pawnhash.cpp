#include "pawnhash.h"
#include <cstring>

/*
| Clear Pawn Hash Table                                                       |
| Resets all stored scores and keys in the pawn hash table to zero.           |
*/
void PawnHashTable::clear() {
  std::memset(mgScores, 0, sizeof(mgScores));
  std::memset(egScores, 0, sizeof(egScores));
  std::memset(keys, 0, sizeof(keys));
}

/*
| Probe Pawn Hash Table                                                       |
| Checks if the pawn structure for the given pawn key has been evaluated      |
| previously. Returns true and populates scores if a match is found.          |
*/
bool PawnHashTable::probe(uint64_t pawnKey, int32_t &mg, int32_t &eg) const {
  int index = pawnKey % PAWN_HASH_SIZE;
  if (keys[index] == pawnKey) {
    mg = mgScores[index];
    eg = egScores[index];
    return true;
  }
  return false;
}

/*
| Store Pawn Evaluation                                                       |
| Saves the evaluated middle-game and end-game scores for a specific pawn     |
| structure into the hash table for future retrieval.                         |
*/
void PawnHashTable::store(uint64_t pawnKey, int32_t mg, int32_t eg) {
  int index = pawnKey % PAWN_HASH_SIZE;
  keys[index] = pawnKey;
  mgScores[index] = mg;
  egScores[index] = eg;
}
