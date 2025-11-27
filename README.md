# ASTROVE Chess Engine

A UCI-compatible chess engine written in C++17.

## Overview

ASTROVE is a chess engine implementing bitboard-based move generation, principal variation search (PVS), and quiescence search. The engine uses a 64MB transposition table with Zobrist hashing for position evaluation.


## Strength

Estimated Rating:  **~1700 Elo**

## Match Results vs Stockfish 1600 elo
```
Score of ASTROVENMP vs ASTROVE: 135 - 10 - 55  [0.813] 200
...      ASTROVENMP playing White: 59 - 3 - 38  [0.780] 100
...      ASTROVENMP playing Black: 76 - 7 - 17  [0.845] 100
...      White vs Black: 66 - 79 - 55  [0.468] 200
Elo difference: 254.7 +/- 46.2, LOS: 100.0 %, DrawRatio: 27.5 %
SPRT: llr 0 (0.0%), lbound -inf, ubound inf


Score of ASTROVENMP vs Stockfish1600: 56 - 34 - 10  [0.610] 100
...      ASTROVENMP playing White: 30 - 15 - 5  [0.650] 50
...      ASTROVENMP playing Black: 26 - 19 - 5  [0.570] 50
...      White vs Black: 49 - 41 - 10  [0.540] 100
Elo difference: 77.7 +/- 66.9, LOS: 99.0 %, DrawRatio: 10.0 %
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

