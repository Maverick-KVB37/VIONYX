#include "../search.h"
#include <algorithm>
#include <cmath>
/*
namespace Search{

    //LMR PRECOMPUTED TABLES
    static int LMRTable[64][64];  // depth,movecount
    static bool LMRInitialized=false;

    //initialized the LMR reduction table
    void Searcher::initLMR(){
        if(LMRInitialized) return;

        for(int depth=1;depth<64;depth++){
            for(int moveCount=1;moveCount<64;moveCount++){
                double reduction=log(depth)*log(moveCount)/2.0;
                LMRTable[depth][moveCount]=static_cast<int>(reduction);
            }
        }
        LMRInitialized=true;
    }

    //get LMR reduction ammount from precomputed table
    int Searcher::getLMRReduction(int depth,int moveCount,bool isPV,bool improving){
        if(!LMRInitialized){
            initLMR();  //ensuring initialization
        }

        if(depth>=64) depth = 63;
        if(moveCount>=64) moveCount=63;

        int reduction=LMRTable[depth][moveCount];

        //adjust reduction based on node type
        if(isPV){
            //reduction=std::max(0,reduction-1);
            return 0;
        }

        //adjust based on improving
        if(!improving){
            reduction+=1; //reducing one more if not improving
        }

        //is reduction is valid
        reduction=std::max(0,reduction);
        reduction=std::min(depth-1,reduction);

        return reduction;
    }

    //to check if move should removed or not
    bool Searcher::shouldReduceMove(int depth,int legalMoves,bool inChcek,
                          bool isCapture,bool isPromotion){
        if(depth<3){
            return false;
        }

        if(legalMoves<=3){
            return false;
        }

        //never reduce tactial moves
        if(isCapture){
            return false;
        }

        if(isPromotion){
            return false;
        }

        //don`t reduce when in check
        if(inChcek){
            return false;
        }

        return true;
    }
}
*/