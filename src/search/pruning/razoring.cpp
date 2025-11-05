#include "../search.h"
/*
namespace Search{

    template <Color c>
    int Searcher::tryRazoring(int alpha,int depth,int ply){
        //so what are skip conditions
        if(depth>3 || depth<0) return NO_RAZOR;
        if(pos.inCheck<c>()) return NO_RAZOR;
        if(ply==0) return NO_RAZOR;

        // Don't razor near mate scores
        if(abs(alpha) > MATE_SCORE - 200) return NO_RAZOR;

        int staticEval=eval.evaluate_board(pos);
        //margins
        const int RAZOR_MARGINS[4]={0,200,300,450};
        int margin=RAZOR_MARGINS[depth];

        //if static eval is way below than alpha then try qsearch
        if(staticEval+margin<alpha){
            int qScore=quiescence<c>(alpha-margin,alpha,ply);
            if(qScore<alpha){
                return qScore;
            }
        }
        return NO_RAZOR;
    }

    //instantinations
    template int Searcher::tryRazoring<White>(int,int,int);
    template int Searcher::tryRazoring<Black>(int,int,int);
}
*/