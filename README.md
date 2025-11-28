# ASTROVE Chess Engine

A UCI-compatible chess engine written in C++17.

## Overview

ASTROVE is a chess engine implementing bitboard-based move generation, principal variation search (PVS), and quiescence search. The engine uses a 64MB transposition table with Zobrist hashing for position evaluation.


## Strength

Estimated Rating:  **~1800 Elo**

## Match Results vs Stockfish 1800 elo
```

Score of ASTROVELMR vs Stockfish1800: 87 - 87 - 26  [0.500] 200
...      ASTROVELMR playing White: 41 - 45 - 14  [0.480] 100
...      ASTROVELMR playing Black: 46 - 42 - 12  [0.520] 100
...      White vs Black: 83 - 91 - 26  [0.480] 200
Elo difference: 0.0 +/- 45.1, LOS: 50.0 %, DrawRatio: 13.0 %
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

