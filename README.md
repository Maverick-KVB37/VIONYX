Astrove/
│
├── src/
│   │
│   ├── core/                           [Foundation - 5 files]
│   │   ├── types.h                     // All types, enums
│   │   ├── bitboard.h                  // Bitboard ops (inline)
│   │   ├── attacks.h/.cpp              // Attack generation
│   │   ├── magic.cpp                   // Magic bitboards
│   │   └── zobrist.cpp                 // Zobrist hashing
│   │
│   ├── board/                          [Board - 5 files]
│   │   ├── position.h                  
│   │   ├── position.cpp                
│   │   ├── movegen.h                   
│   │   ├── movegen.cpp                 
│   │   └── see.h/.cpp                  // Static Exchange Eval
│   │
│   ├── search/                         [Search - 10 files]
│   │   ├── search.h                    
│   │   ├── search.cpp                  // Iterative deepening
│   │   ├── negamax.cpp                 // Main search
│   │   ├── quiescence.cpp              // Q-search
│   │   ├── aspiration.cpp              // Aspiration windows
│   │   ├── ordering.h/.cpp             // Move ordering
│   │   ├── pruning.cpp                 // All pruning techniques
│   │   ├── extensions.cpp              // Extensions
│   │   ├── timemanager.h/.cpp          // Time management
│   │   └── thread.h/.cpp               // Lazy SMP
│   │
│   ├── eval/                           [Evaluation - 12 files]
│   │   ├── evaluate.h                  
│   │   ├── evaluate.cpp                // Main eval dispatcher
│   │   ├── material.cpp                // Material
│   │   ├── psqt.h/.cpp                 // Piece-square tables
│   │   ├── pawns.h/.cpp                // Pawn structure (CRITICAL!)
│   │   ├── king_safety.h/.cpp          // King safety (CRITICAL!)
│   │   ├── mobility.cpp                // Mobility
│   │   ├── pieces.cpp                  // Piece-specific eval
│   │   ├── threats.cpp                 // Threats
│   │   ├── space.cpp                   // Space control
│   │   └── passed_pawns.cpp            // Passed pawn evaluation
│   │
│   ├── table/                          [Tables - 5 files]
│   │   ├── tt.h/.cpp                   // Transposition table
│   │   ├── pawn_cache.h/.cpp           // Pawn hash
│   │   └── material_cache.h/.cpp       // Material hash
│   │
│   ├── endgame/                        [Endgame - 6 files]
│   │   ├── endgame.h/.cpp              // Endgame evaluation
│   │   ├── bitbases.h/.cpp             // Bitbase positions
│   │   └── syzygy.h/.cpp               // Syzygy support
│   │
│   ├── book/                           [Opening - 3 files]
│   │   ├── polyglot.h/.cpp             
│   │   └── book.bin                    
│   │
│   ├── tuning/                         [Tuning - 5 files]
│   │   ├── tuner.h/.cpp                
│   │   ├── texel.h/.cpp                
│   │   └── spsa.cpp                    
│   │
│   ├── uci/                            [UCI - 4 files]
│   │   ├── uci.h/.cpp                  
│   │   └── options.h/.cpp              
│   │
│   ├── utils/                          [Utils - 4 files]
│   │   ├── perft.h/.cpp                
│   │   └── benchmark.h/.cpp            
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
│tuning

├── CMakeLists.txt
└── README.md
