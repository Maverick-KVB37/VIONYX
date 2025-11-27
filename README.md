# ASTROVE Chess Engine

A UCI-compatible chess engine written in C++17.

## Overview

ASTROVE is a chess engine implementing bitboard-based move generation, principal variation search (PVS), and quiescence search. The engine uses a 64MB transposition table with Zobrist hashing for position evaluation.


## Strength

Estimated Rating:  **~1653 Elo**

## Match Results vs Stockfish 1600 elo
```

Score of ASTROVE vs Stockfish1600: 50 - 35 - 15  [0.575] 100
...      ASTROVE playing White: 28 - 15 - 7  [0.630] 50
...      ASTROVE playing Black: 22 - 20 - 8  [0.520] 50
...      White vs Black: 48 - 37 - 15  [0.555] 100
Elo difference: 52.5 +/- 64.0, LOS: 94.8 %, DrawRatio: 15.0 %
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

