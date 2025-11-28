# ASTROVE Chess Engine

A UCI-compatible chess engine written in C++17.

## Overview

ASTROVE is a chess engine implementing bitboard-based move generation, principal variation search (PVS), and quiescence search. The engine uses a 64MB transposition table with Zobrist hashing for position evaluation.


## Strength

Estimated Rating:  **~1842 Elo**

## Match Results vs Stockfish 1800 elo
```

Score of ASTROVEFP vs Stockfish1800: 50 - 38 - 12  [0.560] 100
...      ASTROVEFP playing White: 25 - 18 - 7  [0.570] 50
...      ASTROVEFP playing Black: 25 - 20 - 5  [0.550] 50
...      White vs Black: 45 - 43 - 12  [0.510] 100
Elo difference: 41.9 +/- 65.0, LOS: 90.0 %, DrawRatio: 12.0 %
SPRT: llr 0 (0.0%), lbound -inf, ubound inf


Score of ASTROVEFP vs ASTROVEAW: 35 - 8 - 57  [0.635] 100
...      ASTROVEFP playing White: 14 - 5 - 31  [0.590] 50
...      ASTROVEFP playing Black: 21 - 3 - 26  [0.680] 50
...      White vs Black: 17 - 26 - 57  [0.455] 100
Elo difference: 96.2 +/- 44.1, LOS: 100.0 %, DrawRatio: 57.0 %
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

