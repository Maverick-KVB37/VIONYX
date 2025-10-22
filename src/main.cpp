#include <iostream>
#include <string>
#include <vector>
#include "core/attacks.h"
#include "board/position.h"
#include "board/movegen.h"
#include "core/attacks.h"
#include "core/zobrist.h"
#include "core/types.h"
#include "utils/perft.h"
#include "../src/uci/uci.h"


int main(int argc, char* argv[]) {
    // Set output to unbuffered for immediate communication with GUI
    std::cout.setf(std::ios::unitbuf);
    std::cin.setf(std::ios::unitbuf);
    
        // Initialize magic bitboards
    Astrove::magic::init();
    
    // Initialize Zobrist hashing
    zobrist.init();
    
    // Initialize evaluation tables
    ASTROVE::eval::InitializePieceSquareTable();

    // Create UCI handler
    UCI uci;
    
    // Start UCI loop (handles all communication)
    uci.uciLoop();
    
    return 0;
}

