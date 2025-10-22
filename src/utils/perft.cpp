#include "perft.h"
#include <iostream>
#include <iomanip>
#include <chrono>

U64 perft(Position& pos, int depth, MoveGenerator& gen) {
    if (depth == 0) return 1ULL;

    MoveList moves;
    if (pos.sideToMove() == White) {
        gen.generate_all_moves<White>(pos, moves);
    } else {
        gen.generate_all_moves<Black>(pos, moves);
    }

    U64 nodes = 0;

    for (const Move& move : moves) {
        if (pos.sideToMove() == White) {
            pos.makemove<White>(move);
            if (!pos.isSquareAttacked<Black>(pos.kingsq<White>())) {
                nodes += perft(pos, depth - 1, gen);
            }
            pos.unmakemove<White>(move);
        } else {
            pos.makemove<Black>(move);
            if (!pos.isSquareAttacked<White>(pos.kingsq<Black>())) {
                nodes += perft(pos, depth - 1, gen);
            }
            pos.unmakemove<Black>(move);
        }
    }

    return nodes;
}


void perftDivide(Position& pos, int depth, MoveGenerator& gen) {
    if (depth == 0) return;

    std::cout << "\n--- Perft Divide for FEN: " << pos.toFEN() << " at depth " << depth << " ---\n\n";

    MoveList moves;
    if (pos.sideToMove() == White) {
        gen.generate_all_moves<White>(pos, moves);
    } else {
        gen.generate_all_moves<Black>(pos, moves);
    }

    U64 totalNodes = 0;
    auto start = std::chrono::high_resolution_clock::now();

    for (const Move& move : moves) {
        if (pos.sideToMove() == White) {
            pos.makemove<White>(move);
            if (!pos.isSquareAttacked<Black>(pos.kingsq<White>())) {
                U64 childNodes = perft(pos, depth - 1, gen);
                std::cout << move.to_uci_string() << ": " << childNodes << "\n";
                totalNodes += childNodes;
            }
            pos.unmakemove<White>(move);
        } else {
            pos.makemove<Black>(move);
            if (!pos.isSquareAttacked<White>(pos.kingsq<Black>())) {
                U64 childNodes = perft(pos, depth - 1, gen);
                std::cout << move.to_uci_string() << ": " << childNodes << "\n";
                totalNodes += childNodes;
            }
            pos.unmakemove<Black>(move);
        }
    }

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

    std::cout << "\nTotal nodes: " << totalNodes << "\n";
    std::cout << "Time elapsed: " << duration.count() << " ms\n";
}


void runPerftTests() {
    struct Test {
        std::string fen;
        std::vector<uint64_t> expected;
    };

    std::vector<Test> tests = {
        {"rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1", {20, 400, 8902, 197281, 4865609, 119060324}},
        {"r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1", {48, 2039, 97862, 4085603, 193690690}},
        {"8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1 ", {14, 191, 2812, 43238, 674624, 11030083, 178633661}},
        {"r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1", {6, 264, 9467, 422333, 15833292, 706045033}},
        {"rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ - 1 8", {44, 1486, 62379, 2103487, 89941194}},
        {"r4rk1/1pp1qppp/p1np1n2/2b1p1B1/2B1P1b1/P1NP1N2/1PP1QPPP/R4RK1 w - - 0 10",{46,2079,89890,3894594,164075551}},
        {"8/P1k5/K7/8/8/8/8/8 w - - 0 1",{6,27,273,1329,18135,92683}},
        {"r3k2r/8/3Q4/8/8/5q2/8/R3K2R b KQkq - 0 1",{44,1494,50509,1720476}},
        {"8/k1P5/8/1K6/8/8/8/8 w - - 0 1",{10,25,268,926,10857,43261,567584}},
        {"8/8/2k5/5q2/5n2/8/5K2/8 b - - 0 1",{37,183,6559,23527,811573}},
        {"8/8/4k3/8/2p5/8/B2P2K1/8 w - - 0 1",{13,102,1266,10276,135655,1015133}},
        {"8/8/1P2K3/8/2n5/1q6/8/5k2 b - - 0 1",{29,165,5160,31961,1004658}},
        {"K1k5/8/P7/8/8/8/8/8 w - - 0 1",{2,6,13,63,382,2217}},
        {"3k4/3p4/8/K1P4r/8/8/8/8 b - - 0 1",{18,92,1670,10138,185429,1134888}}
    };

    MoveGenerator gen;
    uint64_t totalNodes = 0;
    auto globalStart = std::chrono::high_resolution_clock::now();
    for (size_t i = 0; i < tests.size(); ++i) {
        std::cout << "Test Position " << (i+1) << ":\n";
        Position pos(tests[i].fen);
        for (size_t d = 0; d < tests[i].expected.size(); ++d) {
            auto start = std::chrono::high_resolution_clock::now();
            uint64_t nodes = perft(pos, d+1, gen);
            auto end = std::chrono::high_resolution_clock::now();
            
            auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
            double timeSeconds = duration.count() / 1000.0;
            uint64_t nps = (timeSeconds > 0) ? (nodes / timeSeconds) : 0;
            totalNodes += nodes;
            std::cout << "  Depth " << (d + 1) << ": " 
                      << std::setw(12) << nodes
                      << " (expected " << std::setw(12) << tests[i].expected[d] << ")";
            if (nodes == tests[i].expected[d])
                std::cout << "  [OK]";
            else
                std::cout << "  [FAIL]";
            
            // Print timing info
            std::cout << " | Time: " << std::setw(8) << std::fixed << std::setprecision(2) 
                      << timeSeconds << "s";
            
            // Print NPS in human-readable format
            if (nps >= 1000000)
                std::cout << " | NPS: " << std::setw(7) << std::fixed << std::setprecision(2) 
                          << (nps / 1000000.0) << " MNPS";
            else if (nps >= 1000)
                std::cout << " | NPS: " << std::setw(7) << std::fixed << std::setprecision(2) 
                          << (nps / 1000.0) << " KNPS";
            else
                std::cout << " | NPS: " << std::setw(7) << nps << " NPS";
            
            std::cout << "\n";
        }
        std::cout << "\n";
    }
    auto globalEnd = std::chrono::high_resolution_clock::now();
    auto globalDuration = std::chrono::duration_cast<std::chrono::milliseconds>(globalEnd - globalStart);
    double totalTimeSeconds = globalDuration.count() / 1000.0;
    uint64_t avgNPS = (totalTimeSeconds > 0) ? (totalNodes / totalTimeSeconds) : 0;
    
    std::cout << "==================== SUMMARY ====================\n";
    std::cout << "Total nodes:   " << std::setw(15) << totalNodes << "\n";
    std::cout << "Total time:    " << std::setw(15) << std::fixed << std::setprecision(2) 
              << totalTimeSeconds << " seconds\n";
    std::cout << "Average NPS:   " << std::setw(15) << std::fixed << std::setprecision(2) 
              << (avgNPS / 1000000.0) << " MNPS\n";
    std::cout << "=================================================\n";
}

