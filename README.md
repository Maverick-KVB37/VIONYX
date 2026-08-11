<p align="center">
  <img src="https://img.shields.io/badge/C%2B%2B-17-blue?style=for-the-badge&logo=cplusplus&logoColor=white" alt="C++17"/>
  <img src="https://img.shields.io/badge/UCI-Compatible-green?style=for-the-badge" alt="UCI"/>
  <img src="https://img.shields.io/badge/Elo-~2750-orange?style=for-the-badge" alt="Elo"/>
  <img src="https://img.shields.io/badge/License-MIT-purple?style=for-the-badge" alt="MIT License"/>
</p>

# ASTROVE

**A high-performance UCI chess engine written from scratch in C++17.**

ASTROVE combines classical alpha-beta search techniques with a hand-crafted evaluation function to deliver strong, principled play. Every line of code — from magic bitboard move generation to tapered evaluation — is written by hand, with no NNUE or external evaluation libraries.

---

## Strength

<table>
<tr>
<td>

### Estimated Rating

# **~2750 Elo**

*Measured via SPRT against ASTROVE 2700*

## Benchmark: History Malus Gravity Correction (ASTROVE 2.0 vs 1.0)

| Metric | Value |
|:---|:---|
| **Matchup** | **ASTROVE 2.0 (Fixed) vs ASTROVE 1.0 (Old)** |
| **Score** | **126W – 71L – 203D** (400 Games) |
| **Win Rate** | **56.9%** |
| **Elo Gain** | **+48.1 ± 23.9** |
| **LOS** | **100.0%** |
| **Draw Ratio** | **50.7%** |

---

### Match Details
* **Time Control:** `8+0.08` (8s + 80ms increment)
* **Opening Suite:** `UHO_2024_8mvs_+090_+099.epd` (UHO 2024)
* **Concurrency:** 8 Threads

```text
Score of ASTROVE_NEW vs ASTROVE_OLD: 126 - 71 - 203  [0.569] 400
...      ASTROVE_NEW playing White: 91 - 17 - 92  [0.685] 200
...      ASTROVE_NEW playing Black: 35 - 54 - 111  [0.453] 200
...      White vs Black: 145 - 52 - 203  [0.616] 400
Elo difference: 48.1 +/- 23.9, LOS: 100.0 %, DrawRatio: 50.7 %
SPRT: llr 0 (0.0%), lbound -inf, ubound inf

```


```
src/
├── main.cpp               # Entry point — unbuffered UCI loop
├── core/                  # Bitboards, types, magic numbers, Zobrist keys
│   ├── attacks.cpp/h      #   Pre-computed attack tables
│   ├── bitboard.cpp/h     #   Bitboard utilities (popcount, LSB, etc.)
│   ├── magic.cpp/h        #   Magic bitboard generation
│   ├── move.cpp/h         #   Move encoding (16-bit compact format)
│   ├── types.h            #   Fundamental types, enums, operators
│   └── zobrist.cpp/h      #   Zobrist hash key generation
├── board/                 # Position representation & move generation
│   ├── position.cpp/h     #   Board state, make/unmake, FEN parsing
│   └── movegen.h          #   Legal move generator
├── search/                # Search algorithm & time management
│   ├── search.cpp/h       #   Iterative deepening + PVS + quiescence
│   └── timemanager.cpp/h  #   Adaptive time allocation
├── evaluation/            # Hand-crafted evaluation (HCE)
│   ├── evaluation.cpp/h   #   Tapered eval with all features
│   └── psqt.cpp/h         #   Piece-square tables
├── ordering/              # Move ordering heuristics
│   └── ordering.cpp/h     #   MVV-LVA, killers, history, countermoves
├── table/                 # Transposition table
│   └── tt.cpp/h           #   64MB TT with aging & bucket system
├── uci/                   # UCI protocol handler
│   └── uci.cpp/h          #   Full UCI implementation
└── utils/                 # Testing & debugging
    ├── perft.cpp/h        #   Perft correctness verification
    └── test_perft.h       #   Perft test positions
```

---

## Features

### Board Representation

| Feature | Details |
|:--|:--|
| **Bitboards** | 64-bit bitboard arrays for each piece type and color |
| **Magic Bitboards** | Pre-computed magic numbers for O(1) slider attack generation |
| **Zobrist Hashing** | Incremental position hashing for TT lookups and repetition detection |
| **Compact Moves** | 16-bit move encoding with special flags for castling, en passant, promotions |

### Search

ASTROVE's search is built on **Principal Variation Search (PVS)** with iterative deepening and a rich set of pruning techniques:

```
IterativeDeepening()
└── pvs<Color, PvNode>(depth, ply, α, β)       ← Negamax with PV/non-PV split
    ├── Transposition Table probe              ← Skip already-explored positions
    ├── Null Move Pruning                      ← R=3 reduction, skip non-PV nodes
    ├── Reverse Futility Pruning               ← Static eval β-cutoff
    ├── Internal Iterative Deepening           ← Find a hash move when TT misses
    ├── Move Ordering                          ← Hash → MVV-LVA → Killers → History → Countermoves
    ├── Late Move Reductions                   ← Reduce depth on late quiet moves
    ├── Late Move Pruning                      ← Skip unpromising quiet moves entirely
    ├── Futility Pruning                       ← Near-leaf α-cutoff for quiet moves
    ├── Check Extensions                       ← Extend when in check
    └── quiescence<Color>(α, β, ply)           ← Tactical resolution at leaf nodes
        ├── Stand Pat evaluation
        └── Capture-only search with SEE ordering
```

#### Move Ordering Priority

| Priority | Technique | Description |
|:-:|:--|:--|
| 1 | **Hash Move** | Best move from the transposition table |
| 2 | **MVV-LVA** | Most Valuable Victim – Least Valuable Attacker |
| 3 | **Killer Moves** | 2 non-capture moves that caused β-cutoffs at the same ply |
| 4 | **History Heuristic** | `history[color][from][to]` — tracks quiet move success |
| 5 | **Counter Move** | `counterMoves[color][from][to]` — replies to opponent's last move |

#### Pruning & Extensions

| Technique | Condition | Effect |
|:--|:--|:--|
| **Null Move Pruning** | Non-PV, non-zugzwang | Skip opponent's turn; if score ≥ β, prune |
| **Late Move Reductions** | Late quiet moves at depth ≥ 3 | Reduce search depth logarithmically |
| **Late Move Pruning** | Non-PV, depth ≤ threshold | Skip quiet moves past move count limit |
| **Reverse Futility Pruning** | Non-PV, static eval ≥ β + margin | Prune entire node |
| **Futility Pruning** | Near leaf, static eval + margin ≤ α | Skip quiet moves |
| **Check Extensions** | King is in check | Extend search by 1 ply |
| **IID** | No hash move available | Search at reduced depth to find one |

### Evaluation (Hand-Crafted)

ASTROVE uses a **tapered evaluation** that smoothly interpolates between middlegame and endgame scores based on remaining material (game phase).

```
final_score = (mg_score × phase + eg_score × (max_phase - phase)) / max_phase
```

| Feature | Middlegame | Endgame | Notes |
|:--|:-:|:-:|:--|
| **Material** | ✔ | ✔ | Tapered piece values (Knights stronger in MG, Rooks in EG) |
| **Piece-Square Tables** | ✔ | ✔ | Separate MG/EG tables for all piece types |
| **Mobility** | ✔ | ✔ | Per-piece bonus tables (Knight: 0–8, Bishop: 0–13, Rook: 0–14) |
| **Isolated Pawns** | −15 | −20 | Penalizes pawns with no adjacent friendly pawns |
| **Doubled Pawns** | −10 | −20 | Penalizes multiple pawns on the same file |
| **Backward Pawns** | −10 | −20 | Penalizes pawns that can't advance safely |
| **Passed Pawns** | +50 | +50 | Massive bonus scaling with rank, dominant in endgames |
| **Knight Outposts** | +25 | +10 | Rank 4–6, pawn-supported, safe from enemy pawns |
| **Bishop Pair** | +30 | +40 | Bonus for retaining both bishops |
| **Rook on Open File** | +20 | +40 | Bonus for rooks on files with no pawns |
| **Rook on Semi-Open** | +10 | +20 | Bonus for rooks on files with only enemy pawns |
| **King Safety** | ✔ | — | Pawn shield bonus + open file penalty, tapered to zero in EG |
| **King Attack Table** | ✔ | — | Weighted attacker count → penalty lookup table (100 entries) |
| **Tempo** | +20 | +10 | Small bonus for the side to move |

### Transposition Table

| Property | Value |
|:--|:--|
| **Default Size** | 64 MB |
| **Entry Size** | 16 bytes (key, score, eval, depth, flag, age, bestmove) |
| **Buckets** | 2 entries per index (best-of-2 replacement) |
| **Replacement** | Depth-preferred with age-based eviction |
| **Hash Full** | Reported via UCI `hashfull` for GUI display |

### UCI Protocol

Full UCI implementation supporting all standard commands:

```
uci • isready • ucinewgame • position [startpos|fen] moves ...
go depth|movetime|wtime|btime|winc|binc|movestogo|infinite|ponder
stop • quit
```

Compatible with **Arena**, **CuteChess**, **Banksia**, **En Croissant**, and all UCI-compliant GUIs.

---

## Building

### Prerequisites

- **C++17** compatible compiler (GCC 7+, Clang 5+, MSVC 2017+)
- **Make** (GNU Make recommended)

### Build Commands

```bash
# Clone the repository
git clone https://github.com/Maverick-KVB37/ASTROVE.git
cd ASTROVE

# Standard optimized build (recommended)
make

# Release build with maximum optimizations
make release

# Debug build with symbols
make debug

# Profile-guided optimization (PGO)
make pgo

# Clean build artifacts
make clean
```

### Build Targets

| Target | Description | Flags |
|:--|:--|:--|
| `make` | Default optimized build | `-O3 -march=native -flto -funroll-loops` |
| `make release` | Maximum optimization | `-O3 -flto -march=native -DNDEBUG` |
| `make debug` | Debug with symbols | `-O0 -g -DDEBUG` |
| `make pgo` | Profile-guided optimization | Two-pass build with profiling data |
| `make profile` | Profiling build | `-O2 -g -pg` |

---

## Usage

### Running the Engine

```bash
# Start the engine in UCI mode
./ASTROVE
```

### Example UCI Session

```
> uci
id name Astrove 2.0
id author Kirti Vardhan Bhushan
uciok

> isready
readyok

> position startpos moves e2e4 e7e5
> go depth 12
info depth 1 score cp 35 nodes 42 nps 420000 pv d2d4
info depth 2 score cp 28 nodes 186 nps 930000 pv d2d4 d7d5
...
info depth 12 score cp 45 nodes 284910 nps 1850000 pv g1f3 b8c6 f1b5
bestmove g1f3

> quit
```

### Testing with CuteChess

```bash
# SPRT test against Stockfish (limited to 2300 Elo)
cutechess-cli \
    -engine name=Astrove cmd=./ASTROVE proto=uci \
    -engine name=SF2300 cmd=./stockfish proto=uci \
        option.UCI_LimitStrength=true option.UCI_Elo=2300 \
        option.Hash=128 option.Threads=1 \
    -each tc=10+0.1 \
    -games 2 -rounds 200 -repeat -recover \
    -concurrency 8 \
    -openings file=noob_3moves.epd format=epd order=random \
    -sprt elo0=0 elo1=10 alpha=0.05 beta=0.05 \
    -pgnout results.pgn
```

---


##  Acknowledgments

ASTROVE's design draws inspiration from the open-source chess programming community:

- [Chess Programming Wiki](https://www.chessprogramming.org/) — The definitive reference
- [Stockfish](https://stockfishchess.org/) — For UCI protocol reference and strength benchmarking
- [CuteChess](https://cutechess.com/) — Engine testing and tournament management
- [Noob Book](https://www.chessprogramming.org/Noob_Book) — Opening book used for testing

---

## License

MIT License — Copyright © 2025 **Kirti Vardhan Bhushan**

See [LICENSE](LICENSE) for full details.

---

<p align="center">
  <i>Built from scratch with ♟ and C++</i>
</p>
