#include "timemanager.h"
#include "search.h"
#include <algorithm>
#include <cmath>

void TimeManager::start(const Search::SearchLimits &limits, Color sideToMove,
                        int moveNumber) {
  startTime = std::chrono::steady_clock::now();
  stopFlag = false;

  timeLeft = (sideToMove == White) ? limits.wtime : limits.btime;
  increment = (sideToMove == White) ? limits.winc : limits.binc;
  movesToGo = limits.movestogo;

  /*
  | Fixed Time Per Move | | If the engine is given a fixed time to search for a
  move, it will           | | search until the exact time is elapsed. |
  */
  if (limits.movetime > 0) {
    timeForMove = limits.movetime;
    stopTime = startTime + std::chrono::milliseconds(timeForMove);
    return;
  }

  /*
  | Infinite Time | | If the search is set to infinite, no time constraints are
  applied,          | | and the engine will search until explicitly stopped. |
  */
  if (limits.infinite) {
    timeForMove = InfiniteTime;
    return;
  }

  /*
  | Fixed Depth (No Time Control) | | If the search specifies a fixed depth
  without any time parameters,          | | the engine will use infinite time to
  complete the requested depth.          |
  */
  if (limits.depth < 128 && timeLeft <= 0 && increment == 0 &&
      !limits.movetime && !limits.infinite) {
    timeForMove = InfiniteTime;
    return;
  }

  /*
  | Dynamic Time Control | | Calculates the time to allocate for the current
  move based on time left,    | | increment, and estimated remaining moves in
  the game.                       |
  */
  if (timeLeft > 0) {

    int64_t estimateMoves =
        estimateMovesRemaining(moveNumber, movesToGo, increment);

    // base time
    timeForMove = timeLeft / estimateMoves;

    // adding increment
    timeForMove += (increment * 3) / 4;

    // safety margin
    int64_t safetyMax = timeLeft - 150;

    if(safetyMax<1){
      safetyMax=std::max<int64_t>(1,timeLeft/2);
    }
    if (timeForMove > safetyMax) {
      timeForMove = safetyMax;
    }

    /*
    | Panic Modes | | If time left drops critically low, we severely constrain
    the time           | | allocated per move to avoid losing on time. |
    */
    if (timeLeft < 10000) {
      timeForMove = std::min(timeForMove, timeLeft / 10);
    }
    if (timeLeft < 3000) {
      timeForMove = std::min(timeForMove, timeLeft / 15);
    }
    if (timeLeft < 1000) {
      timeForMove = std::min(timeForMove, timeLeft / 20);
    }

    // cap (1/6th of remaining time)
    int64_t maxTime = timeLeft / 6;
    if (timeForMove > maxTime) {
      timeForMove = maxTime;
    }

    // min time floor
    if (timeForMove < 50) {
      timeForMove=std::min<int64_t>(50,safetyMax);
  }

    if (timeForMove < 0) {
      timeForMove = 10;
    }

    stopTime = startTime + std::chrono::milliseconds(timeForMove);
  } 
  else{
    // no time info, use default
    timeForMove = 1000;
    stopTime = startTime + std::chrono::milliseconds(timeForMove);
  }
}

int64_t TimeManager::estimateMovesRemaining(int moveNumber, int64_t movesToGo,
                                            int64_t increment) const {

  if (movesToGo > 0) {
    return movesToGo;
  }

  if (increment <= 0) {
    return 40;
  }

  if (moveNumber == 0) {
    return 40;
  }

  const int64_t totalGamePly = 40;
  const int64_t minMovesLeft = 10;

  int64_t remaining = totalGamePly - moveNumber;

  return std::max(minMovesLeft, remaining);
}

void TimeManager::Check() {
  if (stopFlag)
    return;
  if (timeForMove == InfiniteTime)
    return;

  auto now = std::chrono::steady_clock::now();
  if (now >= stopTime) {
    stopFlag = true;
  }
}

int64_t TimeManager::elapsed() const {
  auto now = std::chrono::steady_clock::now();
  auto duration =
      std::chrono::duration_cast<std::chrono::milliseconds>(now - startTime);
  return duration.count();
}
