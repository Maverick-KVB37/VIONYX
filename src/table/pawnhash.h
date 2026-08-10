#pragma once

#include <cstdint>

class PawnHashTable {
public:
  static constexpr int PAWNHASHSIZE=16384; //16K entries

  void clear();
  bool probe(uint64_t pawnKey, int32_t &mg, int32_t &eg) const;
  void store(uint64_t pawnKey, int32_t mg, int32_t eg);

private:
  int32_t mgScores[PAWNHASHSIZE] = {};
  int32_t egScores[PAWNHASHSIZE] = {};
  uint64_t keys[PAWNHASHSIZE] = {};
};
