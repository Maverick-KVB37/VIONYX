// tests/test_hash.cpp
#include "../src/board/position.h"
#include <iostream>
#include <cassert>

void testHashConsistency() {
    Position pos;
    
    // Test 1: Hash should be same after make/unmake
    U64 initialHash = pos.hash();
    
    MoveList moves;
    generateLegalMove<White>(pos, moves);
    
    for (const Move& move : moves) {
        StateInfo state;
        pos.makemove<White>(move);
        
        if (!pos.inCheck()) {
            U64 afterMake = pos.hash();
            
            // Make another random move
            MoveList moves2;
            generateLegalMove<Black>(pos, moves2);
            if (!moves2.empty()) {
                StateInfo state2;
                pos.makemove<Black>(moves2[0]);
                pos.unmakemove<Black>(moves2[0]);
            }
            
            assert(pos.hash() == afterMake);  // Hash should be same
        }
        
        pos.unmakemove<White>(move);
        assert(pos.hash() == initialHash);  // Hash restored!
    }
    
    std::cout << "✓ Hash consistency test passed!\n";
}

void testHashIncremental() {
    Position pos;
    
    // Test that incremental hash matches from-scratch hash
    MoveList moves;
    generateLegalMove<White>(pos, moves);
    
    for (const Move& move : moves) {
        StateInfo state;
        pos.makemove<White>(move);
        
        if (!pos.inCheck()) {
            U64 incrementalHash = pos.hash();
            U64 computedHash = pos.generateHashKey();
            
            if (incrementalHash != computedHash) {
                std::cout << "✗ Hash mismatch after move " << move.to_uci_string() << "\n";
                std::cout << "  Incremental: 0x" << std::hex << incrementalHash << "\n";
                std::cout << "  Computed:    0x" << std::hex << computedHash << "\n";
                assert(false);
            }
        }
        
        pos.unmakemove<White>(move);
    }
    
    std::cout << "✓ Incremental hash test passed!\n";
}
