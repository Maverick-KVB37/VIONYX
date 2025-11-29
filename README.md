# ASTROVE Chess Engine

A UCI-compatible chess engine written in C++17.

## Overview

ASTROVE is a chess engine implementing bitboard-based move generation, principal variation search (PVS), and quiescence search. The engine uses a 64MB transposition table with Zobrist hashing for position evaluation.


## Strength

Estimated Rating:  **~1864 Elo**

## Match Results vs Stockfish 1800 elo
```

Score of ASTROVECM vs Stockfish1800: 100 - 64 - 36  [0.590] 200
...      ASTROVECM playing White: 54 - 28 - 18  [0.630] 100
...      ASTROVECM playing Black: 46 - 36 - 18  [0.550] 100
...      White vs Black: 90 - 74 - 36  [0.540] 200
Elo difference: 63.2 +/- 44.4, LOS: 99.8 %, DrawRatio: 18.0 %
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

