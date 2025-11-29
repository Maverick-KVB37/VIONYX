# ASTROVE Chess Engine

A UCI-compatible chess engine written in C++17.

## Overview

ASTROVE is a chess engine implementing bitboard-based move generation, principal variation search (PVS), and quiescence search. The engine uses a 64MB transposition table with Zobrist hashing for position evaluation.


## Strength

Estimated Rating:  **~1891 Elo**

## Match Results vs Stockfish 1800 elo
```

Score of ASTROVEOP vs Stockfish1800: 111 - 61 - 28  [0.625] 200
...      ASTROVEOP playing White: 53 - 34 - 13  [0.595] 100
...      ASTROVEOP playing Black: 58 - 27 - 15  [0.655] 100
...      White vs Black: 80 - 92 - 28  [0.470] 200
Elo difference: 88.7 +/- 46.1, LOS: 100.0 %, DrawRatio: 14.0 %
SPRT: llr 0 (0.0%), lbound -inf, ubound inf

Score of ASTROVEOP vs ASTROVEIID: 11 - 22 - 67  [0.445] 100
...      ASTROVEOP playing White: 5 - 11 - 34  [0.440] 50
...      ASTROVEOP playing Black: 6 - 11 - 33  [0.450] 50
...      White vs Black: 16 - 17 - 67  [0.495] 100
Elo difference: -38.4 +/- 39.0, LOS: 2.8 %, DrawRatio: 67.0 %
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

