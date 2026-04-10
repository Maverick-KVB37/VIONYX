#include <iostream>
#include "../src/uci/uci.h"


int main(int argc, char* argv[]) {
    // Set output to unbuffered for immediate communication with GUI
    std::cout.setf(std::ios::unitbuf);
    std::cin.setf(std::ios::unitbuf);

    // Create UCI handler (bootEngine handles all initialization)
    UCI uci;
    
    // Start UCI loop (handles all communication)
    uci.uciLoop();
    
    return 0;
}

