#pragma once
#include <chrono>

namespace Search {
    struct SearchLimits;
}

class TimeManager {
public:
    TimeManager()
        : TimeLeft(InfiniteTime),
          Increment(NoValue),
          MovesToGo(NoValue),
          Stop(false),
          hardTimeForMove(NoValue),
          softTimeForMove(NoValue) {}

    // Initialize timer with info from search limits
    void start(const Search::SearchLimits& limits,int sideToMove);

    // Periodically check if time expired and update Stop flag
    void Check();

    // Elapsed time in milliseconds since start
    int elapsed() const;

    // Returns whether time limit or stop condition triggered
    bool StopFlag() const;

    // Soft time cutoff allowance (ms)
    int softTime() const;

private:
    static constexpr int NoValue = 0;
    static constexpr int InfiniteTime = -1;

    int TimeLeft;           // Remaining time for current move (ms)
    int Increment;          // Increment per move (ms)
    int MovesToGo;          // Number of moves before next time control
    bool Stop;              // Flag to indicate search should stop

    std::chrono::time_point<std::chrono::system_clock> startTime; // start timestamp
    std::chrono::time_point<std::chrono::system_clock> stopTime;  // stop timestamp threshold

    int hardTimeForMove; // Hard cutoff for time per move in ms
    int softTimeForMove; // Soft time cutoff in ms (to avoid overshoot)
};