# ASTROVE Chess Engine

A UCI-compatible chess engine written in C++17.

## Overview

ASTROVE is a chess engine implementing bitboard-based move generation, principal variation search (PVS), and quiescence search. The engine uses a 64MB transposition table with Zobrist hashing for position evaluation.


## Strength

Estimated Rating:  **~1520-1530 Elo**

## Match Results vs Stockfish 1700 elo
```
Score of ASTROVE vs SF_1600: 37 - 58 - 5  [0.395] 100
...      ASTROVE playing White: 21 - 28 - 1  [0.430] 50
...      ASTROVE playing Black: 16 - 30 - 4  [0.360] 50
...      White vs Black: 51 - 44 - 5  [0.535] 100
Elo difference: -74.1 +/- 68.7, LOS: 1.6 %, DrawRatio: 5.0 %
SPRT: llr 0 (0.0%), lbound -inf, ubound inf

```

## Features

- Bitboard representation (12 bitboards for piece types)
- Magic bitboard move generation for sliding pieces
- Principal Variation Search (PVS) with alpha-beta pruning
- Quiescence search for tactical positions
- Transposition table (64MB default)
- Move ordering: hash move, MVV-LVA captures, killer moves
- UCI protocol support
- Zobrist hashing for position keys

## Testing

Run test match against Stockfish:



## License

MIT License

Copyright (c) 2025 Kirti Vardhan Bhushan

