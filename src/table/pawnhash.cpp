#include "pawnhash.h"
#include <cstring>

void PawnHashTable::clear() {
    std::memset(mgScores, 0, sizeof(mgScores));
    std::memset(egScores, 0, sizeof(egScores));
    std::memset(keys, 0, sizeof(keys));
}

bool PawnHashTable::probe(uint64_t pawnKey, int32_t& mg, int32_t& eg) const {
    int index = pawnKey % PAWN_HASH_SIZE;
    if (keys[index] == pawnKey) {
        mg = mgScores[index];
        eg = egScores[index];
        return true;
    }
    return false;
}

void PawnHashTable::store(uint64_t pawnKey, int32_t mg, int32_t eg) {
    int index = pawnKey % PAWN_HASH_SIZE;
    keys[index] = pawnKey;
    mgScores[index] = mg;
    egScores[index] = eg;
}
