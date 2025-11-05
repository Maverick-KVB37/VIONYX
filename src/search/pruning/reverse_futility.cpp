#include "../search.h"
/*
namespace Search{

    /**
     * Reverse Futility Pruning(RFP)
     * if static Eval is way above the beta then,
     * our position is so good, that opponent never
     * allow it,so return beta immediately.
     * 
     */
    /*
    template <Color c>
    bool Searcher::tryReverseFutility(int beta,int depth,int ply,int& score){
        // condition to skip RFP
        // only at shallow depth
        if(depth>7||depth<=0) return false;

        //don`t use when in check
        if(pos.inCheck<c>()) return false;

        //don`t use  at root
        if(ply==0) return false;

        //don`t use near mate score
        if(abs(beta)>MATE_SCORE-200) return false;

        int staticEval=eval.evaluate_board(pos);

        //margin based on depth
        const int RFP_MARGINS[8]={0,120,240,360,480,600,720,840};
        int margin=RFP_MARGINS[depth];

        //now check if eval beat beta +amrgin
        if(staticEval-margin>=beta){
            score=beta;
            return true;
        }
        return false;
    }

    //instantiations
    template bool Searcher::tryReverseFutility<White>(int,int,int,int&);
    template bool Searcher::tryReverseFutility<Black>(int,int,int,int&);
}
*/