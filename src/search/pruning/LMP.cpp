#include "../search.h"

namespace Search{

    /**
     * Late Move Pruning(LMP)
     * At shallow depth if w ehave search so many moves,
     * then we can skip remaining quiet move.
     */

    template <Color c>
    bool Searcher::shouldPruneMove(int depth,int moveCount,bool inCheck,
                                   bool isCapture,bool isPromotion,bool givesCheck){
        if(depth>4) return false;
        if(inCheck) return false;

        if(isCapture || isPromotion ||givesCheck) return false;

        //after this many moves prune remaining quiet moves
        const int LMP_THRESHOLDS[5]={0,3,6,12,20};

        if (moveCount>LMP_THRESHOLDS[depth]){
            return true;
        }
        return false;
    }

    template bool Searcher::shouldPruneMove<White>(int,int,bool,bool,bool,bool);
    template bool Searcher::shouldPruneMove<Black>(int,int,bool,bool,bool,bool);
}