//
// Created by Gabriele Gaetano Fronzé on 23/05/2018.
//

#ifndef MTR_SHUTTLE_REPLACEDRPC_H
#define MTR_SHUTTLE_REPLACEDRPC_H

#include <cstdint>
#include <string>
#include "Enumerators.h"

struct ReplacedRPC{
  uint64_t fReplacementTS;
  MTRPlanes fPlane;
  MTRSides fSide;
  MTRRPCs fRPC;
  bool fAlreadyReset = false;

  bool shouldReset(uint64_t currentSOR, MTRPlanes plane, MTRSides side, MTRRPCs RPC, bool accumulating){
    return accumulating && plane==fPlane && side==fSide && RPC==fRPC && currentSOR>fReplacementTS;
  }

  ReplacedRPC(){
    fReplacementTS = 0ULL;
    fPlane = MTRPlanes::kNPlanes;
    fSide = MTRSides::kNSides;
    fRPC = MTRRPCs::kNRPCs;
    fAlreadyReset = false;
  }

  ReplacedRPC(const std::string &str){
    int MT;
    char side;
    int RPC;

    sscanf(str.c_str(),"%lld;MT%d;%c;%d",&fReplacementTS,&MT,&side,&RPC);

    fPlane=MTRPlanes::kNPlanes;

    if(MT==11) fPlane=MTRPlanes::kMT11;
    else if(MT==12) fPlane=MTRPlanes::kMT12;
    else if(MT==21) fPlane=MTRPlanes::kMT21;
    else if(MT==22) fPlane=MTRPlanes::kMT22;

    if(fPlane==MTRPlanes::kNPlanes) printf("The selected plane is not valid: MT{11,12,21,22} %d %d\n",MT,fPlane);

    fSide=MTRSides::kNSides;

    if(side=='O') fSide=MTRSides::kOUTSIDE;
    else if(side=='I') fSide=MTRSides::kINSIDE;

    if(fSide==MTRSides::kNSides) printf("The selected side is not valid: {I,O} %c %d\n",side,fSide);

    fRPC=(MTRRPCs)(RPC-1);
  }
};

#endif //MTR_SHUTTLE_REPLACEDRPC_H
