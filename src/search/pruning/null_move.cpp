#include "../search.h"
/*
namespace Search{

    /*****************************************************
     *  Null Move Pruning(NMP)
     *  so here the concept is if doing nothing(Null Move)
     *  still give me a score>=beta then the position is 
     *  so good that opponent never allow this move so we 
     *  prun this branch
     *****************************************************
     */
    /*
    template <Color c>
    bool Searcher::tryNullMove(int alpha,int beta,int depth,int ply,int& score){

        //don`t do NMP at low depth
        if(depth<3) return false;

        //don`t do NMP when in check
        if(pos.inCheck<c>()) return false;

        //don`t do NMP at root
        if(ply==0) return false;

        if(!pos.hasNonPawnMaterial<c>()) return false;

        //don`t do NMP near mate score
        if(abs(beta)>MATE_SCORE-200) return false;

        //only try NMP if we are ahead
        int staticEval=eval.evaluate_board(pos);
        if(staticEval<beta-100){
            return false;
        }

        //calculate reduction
        int R=2+depth/8;

        //increse R if beta is way good than eval
        if(staticEval>=beta+200) R++;

        if(depth-R-1<1){
            R=depth-2;
        }

        // ================= MAKE NULL MOVE ====================
        Square savedEP=pos.epSquare();
        uint8_t savedHalfMove=pos.getHalfMoveClock();
        uint16_t savedFullMove=pos.getFullMoves();

        pos.makeNullMove<c>();

        //now search at reduced depth with null move
        score = -pvs<~c,false>(depth-R-1,ply+1,-beta,-beta+1,false);

        pos.unmakeNullMove<c>(savedEP,savedHalfMove,savedFullMove);

        //if search is stoped then can`t believe on the score
        if(stopFlag){
            return false;
        }

        //now cutoff
        if(score>=beta){
            if(score>MATE_SCORE-MAX_PLY){
                score=beta;
            }
            return true;
        }
        return false;  //here no cutoff
    }

    template bool Searcher::tryNullMove<White>(int, int, int, int, int&);
    template bool Searcher::tryNullMove<Black>(int, int, int, int, int&);
}
*/