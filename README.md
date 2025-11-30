# ASTROVE Chess Engine

A UCI-compatible chess engine written in C++17.

## Overview

ASTROVE is a chess engine implementing bitboard-based move generation, principal variation search (PVS), and quiescence search. The engine uses a 64MB transposition table with Zobrist hashing for position evaluation.


## Strength

Estimated Rating:  **~1914 Elo**(VS Stockfish 1800 benchmark)

## Match Results vs Stockfish 1800 elo
```

Score of ASTROVELMRTA vs Stockfish1800: 116 - 53 - 31  [0.657] 200
...      ASTROVELMRTA playing White: 60 - 23 - 17  [0.685] 100
...      ASTROVELMRTA playing Black: 56 - 30 - 14  [0.630] 100
...      White vs Black: 90 - 79 - 31  [0.527] 200
Elo difference: 113.3 +/- 46.5, LOS: 100.0 %, DrawRatio: 15.5 %
SPRT: llr 0 (0.0%), lbound -inf, ubound inf


```


---

##  Features

### 1.  Core 

- **Bitboard Representation (64-bit)**
- **Magic Bitboards**
- **Zobrist Hashing**
- **Transposition Table**

- **UCI Protocol Support**
  - Full support for commands like:
    ```
    uci, isready, ucinewgame, position, go, stop, quit
    ```
  - Compatible with all UCI GUIs.

---

### 2.  SEARCH

  - **Iterative Deepening**
  - **Principal Variation Search (PVS)**
  - **Alpha-Beta pruning**

#### Move Ordering Priority

1. **Hash Move** (best from TT)
2. **MVV-LVA** (Best captures first)
3. **Killer Heuristic** (2 killer moves per ply)
4. **History Heuristic** (Quiet move success based ordering)
5. **Counter Move History** (Replies to opponent move)

#### Pruning & Selectivity

- **NMP (Null Move Pruning)**
- **LMR (Late Move Reduction)**
- **LMP (Late Move Pruning)**
- **RFP (Reverse Futility Pruning)** 
- **FP (Futility Pruning)**
- **IID (Internal Iterative Deepening)**  
- **Check Extensions** 
- **Quiescence Search**

---

### 3.Evaluation (Hand Crafted Evaluation – HCE)

#### Evaluation Components

| Feature | Description |
|--------|-------------|
| **Material** | Tapered values (Knights strong in MG, Rooks in EG) |
| **Isolated Pawns** | Penalized |
| **Doubled Pawns** | Penalized |
| **Passed Pawns** | Big bonus increasing with rank, massive in endgames |
| **Knight Outposts** | Bonus rank 4-6 if supported by pawn, safe from enemy pawns |
| **Bishop Pair** | Static bonus |
| **Rooks on files** | Bonus for open and semi-open files |
| **King Safety** | Pawn shield + open file penalty (reduced to 0 in endgame) |

King safety is **tapered to zero in endgame** to force king activation.

---

## Testing

### ▶ Run a quick match

```bash
cutechess-cli -engine cmd=./ASTROVE -engine cmd=./Stockfish1800 -each tc=10+0.1 -rounds 100
```

## License

MIT License

Copyright (c) 2025 Kirti Vardhan Bhushan

