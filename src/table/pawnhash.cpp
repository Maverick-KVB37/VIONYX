#include "pawnhash.h"
#include <cstring>

/*
| Clear Pawn Hash Table                                                       |
| Resets all stored scores and keys in the pawn hash table to zero.           |
*/
void PawnHashTable::clear() {
  std::memset(mgScores, 0, sizeof(mgScores));
  std::memset(egScores, 0, sizeof(egScores));
  std::memset(keys, 0, sizeof(keys));
}

/*
| Probe Pawn Hash Table                                                       |
| Checks two slots to see if the pawn structure has been evaluated            |
| previously. Returns true and populates scores if a match is found           |
*/
bool PawnHashTable::probe(uint64_t pawnKey, int32_t &mg, int32_t &eg) const {
  int idx1=pawnKey%PAWNHASHSIZE;
  int idx2=(idx1+1)%PAWNHASHSIZE; //second slot for collision

  //first slot
  if(keys[idx1]==pawnKey){
    mg = mgScores[idx1];
    eg = egScores[idx1];
    return true;
  }

  //second slot
  if(keys[idx2]==pawnKey){
    mg = mgScores[idx2];
    eg = egScores[idx2];
    return true;
  }

  return false;
}


/*
| Store Pawn Evaluation                                                       |
| Saves the evaluated scores using a 2 way collision resolution strategy      |
*/
void PawnHashTable::store(uint64_t pawnKey, int32_t mg, int32_t eg) {
  int idx1=pawnKey%PAWNHASHSIZE;
  int idx2=(idx1+1)%PAWNHASHSIZE;

  //prevent redundant write if it is already in the table
  if (keys[idx1]==pawnKey || keys[idx2]==pawnKey) {
    return;
  }

  //prefer empty slot first to avoid kick out useful data
  if(keys[idx1]==0){
    keys[idx1]=pawnKey;
    mgScores[idx1]=mg;
    egScores[idx1]=eg;
    return;
  }
  if(keys[idx2]==0){
    keys[idx2]=pawnKey;
    mgScores[idx2]=mg;
    egScores[idx2]=eg;
    return;
  }

  //if both slot are full overwrite the first one
  keys[idx1]=pawnKey;
  mgScores[idx1]=mg;
  egScores[idx1]=eg;
}
