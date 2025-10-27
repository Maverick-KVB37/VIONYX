#include "timemanager.h"
#include <algorithm>
#include "search.h"
void TimeManager::start(const Search::SearchLimits& limits,int sideToMove) {
    startTime = std::chrono::system_clock::now();
    Stop = false;
    
    TimeLeft = limits.time[sideToMove];
    Increment = limits.inc[sideToMove];
    MovesToGo = limits.movestogo;
    
    if (limits.movetime > 0) {
        //for (g0 movetime 1000)
        hardTimeForMove = limits.movetime;
        softTimeForMove = limits.movetime;
    } else if (limits.infinite) {
        hardTimeForMove = 1000000;
        softTimeForMove = 1000000;
    } else if (TimeLeft > 0) {
        // Simple time allocation heuristic
        int estimatedMovesLeft = (MovesToGo > 0) ? MovesToGo : 30;
        //divide times by remaining times
        int basetime=TimeLeft/estimatedMovesLeft;
        int incrementbonus=(Increment*3)/4;

        softTimeForMove = basetime+incrementbonus;
        int maxtimeallowed=TimeLeft/4;
        
        hardTimeForMove = std::min(maxtimeallowed, softTimeForMove * 4);

        softTimeForMove=std::max(softTimeForMove,50);
        hardTimeForMove=std::max(hardTimeForMove,100);

        //check for low time let`s say <5sec
        if(TimeLeft<5000){
            softTimeForMove=std::min(softTimeForMove,TimeLeft/10);
            hardTimeForMove=std::min(hardTimeForMove,TimeLeft/4);
        }

        //and for <1sec
        if(TimeLeft<1000){
            softTimeForMove=std::min(softTimeForMove,TimeLeft/20);
            hardTimeForMove=std::min(hardTimeForMove,TimeLeft/10);
        }

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
