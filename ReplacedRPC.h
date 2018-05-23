//
// Created by Gabriele Gaetano Fronzé on 23/05/2018.
//

#ifndef MTR_SHUTTLE_REPLACEDRPC_H
#define MTR_SHUTTLE_REPLACEDRPC_H

struct ReplacedRPC{
  uint64_t fReplacementTS;
  MTRPlanes fPlane;
  MTRSides fSide;
  MTRRPCs fRPC;
  bool fAlreadyReset = false;

  bool shouldReset(uint64_t currentSOR, MTRPlanes plane, MTRSides side, MTRRPCs RPC, bool accumulating){
    fAlreadyReset = accumulating && plane==fPlane && side==fSide && RPC==fRPC && currentSOR>fReplacementTS;
    return fAlreadyReset;
  }

  ReplacedRPC(const std::string &str){
    int MT;
    char side;
    int RPC;

    sscanf(str.c_str(),"%lld;MT%2d;%c;%1d",&fReplacementTS,&MT,&side,&RPC);

    switch (MT){
      case 11: fPlane=MTRPlanes::kMT11;
      case 12: fPlane=MTRPlanes::kMT12;
      case 21: fPlane=MTRPlanes::kMT21;
      case 22: fPlane=MTRPlanes::kMT22;
      default: fPlane=MTRPlanes::kNPlanes; //This won't ever match
    }
    if(fPlane==MTRPlanes::kNPlanes) printf("The selected plane is not valid: MT{11,12,21,22}\n");

    switch (side) {
      case 'I': fSide=MTRSides::kINSIDE;
      case 'O': fSide=MTRSides::kOUTSIDE;
      default : fSide=MTRSides::kNSides; //This won't ever match
    }
    if(fSide==MTRSides::kNSides) printf("The selected side is not valid: {I,O}\n");

    fRPC=(MTRRPCs)(RPC-1);
  }
};

#endif //MTR_SHUTTLE_REPLACEDRPC_H
