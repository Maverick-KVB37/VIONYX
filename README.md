# Astrove Chess Engine

## Project Structure

```text
Astrove/
├── src/
│   ├── core/                  # Foundation
│   │   ├── types.h
│   │   ├── bitboard.h
│   │   ├── attacks.h
│   │   ├── attacks.cpp
│   │   ├── magic.cpp
│   │   └── zobrist.cpp
│   │
│   ├── board/
│   │   ├── position.h
│   │   ├── position.cpp
│   │   ├── movegen.h
│   │   ├── movegen.cpp
│   │   ├── see.h
│   │   └── see.cpp
│   │
│   ├── search/
│   │   ├── search.h
│   │   ├── search.cpp
│   │   ├── negamax.cpp
│   │   ├── quiescence.cpp
│   │   ├── aspiration.cpp
│   │   ├── ordering.h
│   │   ├── ordering.cpp
│   │   ├── pruning.cpp
│   │   ├── extensions.cpp
│   │   ├── timemanager.h
│   │   ├── timemanager.cpp
│   │   ├── thread.h
│   │   └── thread.cpp
│   │
│   ├── eval/
│   │   ├── evaluate.h
│   │   ├── evaluate.cpp
│   │   ├── material.cpp
│   │   ├── psqt.h
│   │   ├── psqt.cpp
│   │   ├── pawns.h
│   │   ├── pawns.cpp
│   │   ├── king_safety.h
│   │   ├── king_safety.cpp
│   │   ├── mobility.cpp
│   │   ├── pieces.cpp
│   │   ├── threats.cpp
│   │   ├── space.cpp
│   │   └── passed_pawns.cpp
│   │
│   ├── table/
│   │   ├── tt.h
│   │   ├── tt.cpp
│   │   ├── pawn_cache.h
│   │   ├── pawn_cache.cpp
│   │   ├── material_cache.h
│   │   └── material_cache.cpp
│   │
│   ├── endgame/
│   │   ├── endgame.h
│   │   ├── endgame.cpp
│   │   ├── bitbases.h
│   │   ├── bitbases.cpp
│   │   ├── syzygy.h
│   │   └── syzygy.cpp
│   │
│   ├── book/
│   │   ├── polyglot.h
│   │   ├── polyglot.cpp
│   │   └── book.bin
│   │
│   ├── tuning/
│   │   ├── tuner.h
│   │   ├── tuner.cpp
│   │   ├── texel.h
│   │   ├── texel.cpp
│   │   └── spsa.cpp
│   │
│   ├── uci/
│   │   ├── uci.h
│   │   ├── uci.cpp
│   │   ├── options.h
│   │   └── options.cpp
│   │
│   ├── utils/
│   │   ├── perft.h
│   │   ├── perft.cpp
│   │   ├── benchmark.h
│   │   └── benchmark.cpp
│   │
│   └── main.cpp
│
├── data/
│   ├── books/
│   │   └── performance.bin
│   ├── syzygy/
│   │   └── 3-7piece/
│   └── tuning/
│       └── quiet-labeled.epd
│
├── CMakeLists.txt
└── README.md
