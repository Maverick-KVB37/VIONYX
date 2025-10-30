#pragma once
#include <chrono>
#include "../core/types.h"

namespace Search {
    struct SearchLimits;
}

class TimeManager {
public:
    TimeManager()
        : timeLeft(0),
          increment(0),
          movesToGo(0),
          timeForMove(0),
          stopFlag(false) {}

    // Initialize timer with info from search limits
    void start(const Search::SearchLimits& limits,Color sideToMove,int moveNumber=0);

    // Periodically check if time expired and update Stop flag
    void Check();

    // Elapsed time in milliseconds since start
    int64_t elapsed() const;

    // Returns whether time limit or stop condition triggered
    bool StopFlag() const { return stopFlag; }

    int64_t allocatedTime() const { return timeForMove; }

private:
    static constexpr int NoValue = 0;
    static constexpr int InfiniteTime = -1;

    int64_t timeLeft;           // Remaining time for current move (ms)
    int64_t increment;          // Increment per move (ms)
    int64_t movesToGo;          // Number of moves before next time control
    int64_t timeForMove;
    bool stopFlag;              // Flag to indicate search should stop

    std::chrono::steady_clock::time_point startTime;
    std::chrono::steady_clock::time_point stopTime;

    int64_t estimateMovesRemaining(int moveNumber, int64_t movesToGo, int64_t increment) const;
};