#include <iostream>
#include<cassert>
#include "../board/position.h"
void testFENParsing() {
    struct FENTest {
        std::string fen;
        int expectedWhitePawns;
        int expectedBlackPawns;
        Color expectedSide;
    };
    
    FENTest tests[] = {
        {"rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1", 8, 8, White},
        {"4k3/8/8/8/8/8/4P3/4K3 w - - 0 1", 1, 0, White},
        {"4k3/8/8/8/8/8/8/4K3 b - - 0 1", 0, 0, Black},
    };
    
    for (const auto& test : tests) {
        Position pos(test.fen);
        
        // Count pieces
        int whitePawns = popcount(pos.pawns<White>());
        int blackPawns = popcount(pos.pawns<Black>());
        
        assert(whitePawns == test.expectedWhitePawns);
        assert(blackPawns == test.expectedBlackPawns);
        assert(pos.sideToMove() == test.expectedSide);
        
        // Test round-trip
        std::string outputFEN = pos.toFEN();
        Position pos2(outputFEN);
        assert(pos.hash() == pos2.hash());
    }
    
    std::cout << "✓ FEN parsing test passed!\n";
}
