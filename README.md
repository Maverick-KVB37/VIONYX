# ASTROVE Chess Engine

A UCI-compatible chess engine written in C++17.

## Overview

ASTROVE is a chess engine implementing bitboard-based move generation, principal variation search (PVS), and quiescence search. The engine uses a 64MB transposition table with Zobrist hashing for position evaluation.


## Strength

Estimated Rating:  **~1891 Elo**

## Match Results vs Stockfish 1800 elo
```

Score of ASTROVEKS vs Stockfish1800: 113 - 62 - 25  [0.627] 200
...      ASTROVEKS playing White: 59 - 32 - 9  [0.635] 100
...      ASTROVEKS playing Black: 54 - 30 - 16  [0.620] 100
...      White vs Black: 89 - 86 - 25  [0.507] 200
Elo difference: 90.6 +/- 46.6, LOS: 100.0 %, DrawRatio: 12.5 %
SPRT: llr 0 (0.0%), lbound -inf, ubound inf

Score of ASTROVEIID vs ASTROVELMP: 15 - 11 - 74  [0.520] 100
...      ASTROVEIID playing White: 4 - 5 - 41  [0.490] 50
...      ASTROVEIID playing Black: 11 - 6 - 33  [0.550] 50
...      White vs Black: 10 - 16 - 74  [0.470] 100
Elo difference: 13.9 +/- 34.7, LOS: 78.4 %, DrawRatio: 74.0 %
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

