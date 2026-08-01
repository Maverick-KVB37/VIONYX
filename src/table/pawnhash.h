#pragma once

#include <cstdint>

class PawnHashTable {
public:
  static constexpr int PAWN_HASH_SIZE = 16384; // 16K entries

  void clear();
  bool probe(uint64_t pawnKey, int32_t &mg, int32_t &eg) const;
  void store(uint64_t pawnKey, int32_t mg, int32_t eg);

private:
  int32_t mgScores[PAWN_HASH_SIZE] = {};
  int32_t egScores[PAWN_HASH_SIZE] = {};
  uint64_t keys[PAWN_HASH_SIZE] = {};
};
