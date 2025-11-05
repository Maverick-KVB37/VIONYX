# ASTROVE Chess Engine

A UCI-compatible chess engine written in C++17.

## Overview

ASTROVE is a chess engine implementing bitboard-based move generation, principal variation search (PVS), and quiescence search. The engine uses a 64MB transposition table with Zobrist hashing for position evaluation.


## Strength

Estimated Rating:  **~1586 Elo**

## Match Results vs Stockfish 1700 elo
```

Score of ASTROVE_timemanager vs ASTROVE_baseline: 93 - 24 - 283  [0.586] 400
...      ASTROVE_timemanager playing White: 60 - 12 - 128  [0.620] 200
...      ASTROVE_timemanager playing Black: 33 - 12 - 155  [0.552] 200
...      White vs Black: 72 - 45 - 283  [0.534] 400
Elo difference: 60.5 +/- 18.0, LOS: 100.0 %, DrawRatio: 70.8 %
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

