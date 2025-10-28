#include "../search.h"

namespace Search{

    template <Color c>
    bool Searcher::canFutilityPrune(int alpha,int depth,int staticEval,
                                    bool isCapture,bool isPromotion,bool givesCheck){
        // condition to skip FP
        // only at shallow depth
        if(depth>3||depth<=0) return false;

        //don`t prune tactical moves
        if(isCapture||isPromotion||givesCheck) return false;

        //margin based on depth
        const int FUTILITY_MARGINS[4]={0,150,300,500};
        int margin=FUTILITY_MARGINS[depth];

        //now check if eval beat beta +amrgin
        if(staticEval+margin<=alpha){
            return true;
        }
        return false;
    }

    //instantiations
    template bool Searcher::canFutilityPrune<White>(int,int,int,bool,bool,bool);
    template bool Searcher::canFutilityPrune<Black>(int,int,int,bool,bool,bool);
}