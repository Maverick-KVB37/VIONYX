#pragma once
#include <string>
#include "../board/position.h"
#include "../board/movegen.h"

// Count all leaf nodes at given depth
U64 perft(Position& pos, int depth,MoveGenerator& gen);

// Perft with move breakdown (for debugging)
void perftDivide(Position& pos, int depth,MoveGenerator& gen);

// Run test suite
void runPerftTests();