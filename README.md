# ASTROVE Chess Engine

A UCI-compatible chess engine written in C++17.

## Overview

ASTROVE is a chess engine implementing bitboard-based move generation, principal variation search (PVS), and quiescence search. The engine uses a 64MB transposition table with Zobrist hashing for position evaluation.


## Strength

Estimated rating: **~1427 Elo**

### Match Results vs Stockfish 1320 Elo
```
Score of ASTROVE v1.0 vs Stockfish 1320: 32 - 17 - 1 [0.650] 50
... ASTROVE v1.0 playing White: 16 - 8 - 1 [0.660] 25
... ASTROVE v1.0 playing Black: 16 - 9 - 0 [0.640] 25
... White vs Black: 25 - 24 - 1 [0.510] 50
Elo difference: 107.5 +/- 103.5, LOS: 98.4%, DrawRatio: 2.0%
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

