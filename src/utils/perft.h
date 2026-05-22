#pragma once
#include <string>
#include "../board/position.h"
#include "../board/movegen.h"

U64 perft(Position& pos, int depth,MoveGenerator& gen);

//perft with move breakdown (for debugging)
void perftDivide(Position& pos, int depth,MoveGenerator& gen);

void runPerftTests();