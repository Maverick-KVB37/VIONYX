<p align="center">
  <img src="https://img.shields.io/badge/C%2B%2B-17-blue?style=for-the-badge&logo=cplusplus&logoColor=white" alt="C++17"/>
  <img src="https://img.shields.io/badge/UCI-Compatible-green?style=for-the-badge" alt="UCI"/>
  <img src="https://img.shields.io/badge/Elo-~2508-orange?style=for-the-badge" alt="Elo"/>
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

# **~2597 Elo**

*Measured via SPRT against SF2500 baseline*

</td>
<td>

| Metric | Value |
|:--|:--|
| **Score vs SF2500** | **208W – 111L – 36D** |
| **Win Rate** | **63.7%** |
| **Elo Difference** | **+97.4 ± 35.6** |
| **LOS** | **100.0%** |
| **SPRT Result** |  **H1 Accepted** |

</td>
</tr>
</table>

---

## Benchmark: C++ & MoveGen Speed Optimizations

355 games · Time Control `10+0.1` · Opening Book `noob_3moves.epd` · SPRT `elo0=0 elo1=10 α=0.05 β=0.05`

```text
Score of Astrove vs SF2500: 208 - 111 - 36  [0.637] 355
...      Astrove playing White: 97 - 59 - 21  [0.607] 177
...      Astrove playing Black: 111 - 52 - 15  [0.666] 178
...      White vs Black: 149 - 170 - 36  [0.470] 355
Elo difference: 97.4 +/- 35.6, LOS: 100.0 %, DrawRatio: 10.1 %
SPRT: llr 2.97 (101.0%), lbound -2.94, ubound 2.94 - H1 was accepted
```

<details>
<summary><b>Detailed Game Termination Statistics</b></summary>

| Termination Reason | Astrove_NEW | Astrove_OLD |
|:--|:-:|:-:|
| Win: White mates | 39 | 15 |
| Win: Black mates | 49 | 14 |
| Loss: White mates | 15 | 39 |
| Loss: Black mates | 14 | 49 |
| Draw by 3-fold repetition | 57 | 57 |
| Draw by fifty moves rule | 4 | 4 |
| Draw by insufficient material | 19 | 19 |
| Draw by stalemate | 1 | 1 |
| No result | 2 | 2 |

</details>

### Phase 2 Improvements (Speed & Memory Optimizations)
* **Zero-Heap Allocations:** Replaced all instances of `std::vector` with fixed-size stack arrays for `MoveList`, move ordering `scores`, and `positionHistory`. This completely eliminates dynamic memory allocation overhead during the search.
* **Quiescence Move Generation:** Implemented a dedicated `generate_captures()` function. Quiescence search now directly generates only captures and Queen promotions, entirely skipping the overhead of generating and filtering out quiet moves.
* **Link-Time Optimization (LTO):** Corrected `Makefile` linking stage flags (`LDFLAGS = -flto`) to ensure aggressive cross-file function inlining, drastically reducing per-node function call overhead.

## Architecture

ASTROVE is a modular, cleanly structured engine with **~5,000 lines of C++17** across **37 source files**.

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
│   ├── movegen.h          #   Legal move generator
│   └── see.cpp/h          #   Static Exchange Evaluation
├── search/                # Search algorithm & time management
│   ├── search.cpp/h       #   Iterative deepening + PVS + quiescence
│   ├── timemanager.cpp/h  #   Adaptive time allocation
│   └── thread.cpp/h       #   Thread management
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
iterative_deepening()
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
| **Passed Pawns** | +5…+150 | +10…+240 | Massive bonus scaling with rank, dominant in endgames |
| **Knight Outposts** | +10…+40 | +5…+20 | Rank 4–6, pawn-supported, safe from enemy pawns |
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
git clone https://github.com/yourusername/ASTROVE.git
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
id name ASTROVE
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
