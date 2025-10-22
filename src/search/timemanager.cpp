#include "timemanager.h"
#include <algorithm>
#include "search.h"
void TimeManager::start(const Search::SearchLimits& limits) {
    startTime = std::chrono::system_clock::now();
    Stop = false;
    
    TimeLeft = limits.time[0]; // Adjust based on side to move if needed
    Increment = limits.inc[0];
    MovesToGo = limits.movestogo;
    
    if (limits.movetime > 0) {
        hardTimeForMove = limits.movetime;
        softTimeForMove = limits.movetime;
    } else if (limits.infinite) {
        hardTimeForMove = 1000000;
        softTimeForMove = 1000000;
    } else if (TimeLeft > 0) {
        // Simple time allocation heuristic
        int movesToGo = (MovesToGo > 0) ? MovesToGo : 30;
        softTimeForMove = TimeLeft / movesToGo + Increment / 2;
        hardTimeForMove = std::min(TimeLeft / 3, softTimeForMove * 3);
    } else {
        softTimeForMove = 1000000;
        hardTimeForMove = 1000000;
    }
    
    stopTime = startTime + std::chrono::milliseconds(hardTimeForMove);
}

void TimeManager::Check() {
    auto now = std::chrono::system_clock::now();
    if (now >= stopTime) {
        Stop = true;
    }
}

int TimeManager::elapsed() const {
    auto now = std::chrono::system_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(now - startTime);
    return static_cast<int>(duration.count());
}

bool TimeManager::StopFlag() const {
    return Stop;
}

int TimeManager::softTime() const {
    return softTimeForMove;
}
