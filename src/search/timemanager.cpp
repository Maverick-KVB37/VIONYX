#include "timemanager.h"
#include <algorithm>
#include "search.h"
#include <cmath>

void TimeManager::start(const Search::SearchLimits& limits,Color sideToMove, int moveNumber) {
    startTime = std::chrono::steady_clock::now();
    stopFlag = false;
    
    timeLeft = (sideToMove == White) ? limits.wtime : limits.btime;
    increment = (sideToMove == White) ? limits.winc : limits.binc;
    movesToGo = limits.movestogo;
    
    // DEBUG
    /*std::cerr << "TM: timeLeft=" << timeLeft 
              << " inc=" << increment 
              << " movestogo=" << movesToGo 
              << " moveNumber=" << moveNumber 
              << " infinite=" << limits.infinite 
              << " depth=" << limits.depth << std::endl;
    */

    //fixed time per move
    if(limits.movetime>0){
        timeForMove=limits.movetime;
        stopTime=startTime+std::chrono::milliseconds(timeForMove);
        //std::cerr << "TM: Using movetime=" << timeForMove << "ms" << std::endl;
        return;
    }

    //infinite time
    if(limits.infinite){
        timeForMove=InfiniteTime;
        //std::cerr << "TM: Using infinite time" << std::endl;
        return;
    }

    //if a fixed depth is requested and no time control
    if(limits.depth<128 && timeLeft<=0 && increment==0 && !limits.movetime && !limits.infinite){
        timeForMove=InfiniteTime;
        return;
    }

    //time control
    if(timeLeft>0){

        //estimate moves remaining
        int64_t estimateMoves=estimateMovesRemaining(moveNumber,movesToGo,increment);
        //std::cerr << "TM: Estimated moves=" << estimateMoves << std::endl;

        //base time
        timeForMove=timeLeft/estimateMoves;

        //adding increment
        timeForMove+=(increment*3)/4;

        //safety margin
        int64_t safetyMax=timeLeft-150;
        if(timeForMove>safetyMax){
            timeForMove=safetyMax;
        }

        //pamic modes
        if(timeLeft<10000){
            timeForMove=std::min(timeForMove,timeLeft/10);
        }
        if(timeLeft<3000){
            timeForMove=std::min(timeForMove,timeLeft/15);
        }
        if(timeLeft<1000){
            timeForMove=std::min(timeForMove,timeLeft/20);
        }

        //cap(1/6th of remaining time)
        int64_t maxTime=timeLeft/6;
        if(timeForMove>maxTime){
            timeForMove=maxTime;
        }

        // min time floor
        if(timeForMove<50){
            timeForMove=50;
        }

        //sanity check
        if(timeForMove<0){
            timeForMove=10;
        }

        //std::cerr << "TM: Final allocated time=" << timeForMove << "ms" << std::endl;
        stopTime = startTime + std::chrono::milliseconds(timeForMove);
    }
    else {
        //no time info
        timeForMove=1000;
        //std::cerr << "TM: No time info, using default 1000ms" << std::endl;
        stopTime = startTime + std::chrono::milliseconds(timeForMove);
    }   
}

//estimation of no of moves remaining
int64_t TimeManager::estimateMovesRemaining(int moveNumber,int64_t movesToGo, int64_t increment) const {

    //if movestogo specified we should use
    if(movesToGo>0){
        return movesToGo;
    }

    if (increment<=0) {
        return 40;
    }

    //if movetogo is not then we can use default 
    if(moveNumber==0){
        return 40;
    }

    const int64_t totalGamePly=40;
    const int64_t minMovesLeft=10;

    int64_t remaining=totalGamePly-moveNumber;
    
    return std::max(minMovesLeft,remaining);
}


void TimeManager::Check() {
    if(stopFlag) return;
    if(timeForMove==InfiniteTime) return;

    auto now= std::chrono::steady_clock::now();
    if(now>=stopTime){
        stopFlag=true;
    }
}

int64_t TimeManager::elapsed() const {
    auto now = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(now - startTime);
    return duration.count();
}
