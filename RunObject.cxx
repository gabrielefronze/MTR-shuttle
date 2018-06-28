//
// Created by Gabriele Gaetano Fronzé on 01/02/2018.
//

#include "RunObject.h"

RunObject::RunObject(uint64_t SOR, uint64_t EOR, double avgHV, double avgITot, double avgIDark, double intCharge,
                     uint64_t scalBending, uint64_t scalNotBending, bool isDark, bool isDummy)
: fSOR(SOR), fEOR(EOR), fIsDark(isDark), fIsDummy(isDummy)
{
  setAvgHV(avgHV);
  setAvgITot(avgITot);
  setAvgIDark(avgIDark);
  setIntCharge(intCharge);
  setScalBending(scalBending);
  setScalNotBending(scalNotBending);
}

RunObject::RunObject(uint64_t SOR, uint64_t EOR, double avgHV[totRPCN], double avgITot[totRPCN],
                     double avgIDark[totRPCN], double intCharge[totRPCN], uint64_t scalBending[totRPCN],
                     uint64_t scalNotBending[totRPCN], bool isDark, bool isDummy)
  : fSOR(SOR), fEOR(EOR), fIsDark(isDark), fIsDummy(isDummy)
{
  std::memmove(fAvgHV, avgHV, totRPCN);
  std::memmove(fAvgITot, avgITot, totRPCN);
  std::memmove(fAvgIDark, avgIDark, totRPCN);
  std::memmove(fIntCharge, intCharge, totRPCN);
  std::memmove(fScalBending, scalBending, totRPCN);
  std::memmove(fScalNotBending, scalNotBending, totRPCN);
}

// RunObject::RunObject(std::string csvLine, int &plane, int &side, int &RPC)
//{
//  int isDarkBuffer = 0;
//  int isHVOkBuffer = 0;
//  int isDummyBuffer = 0;
//  sscanf(csvLine.c_str(),"%d;%d;%d;%llu;%llu;%llu;%lf;%lf;%lf;%lf;%lf;%lf;%d;%d;%d",
//         &plane,
//         &side,
//         &RPC,
//         &fRunNumber,
//         &fSOR,
//         &fEOR,
//         &fAvgHV,
//         &fAvgITot,
//         &fAvgIDark,
//         &fIntCharge,
//         &fScalBending,
//         &fScalNotBending,
//         &isDarkBuffer,
//         &isHVOkBuffer,
//         &isDummyBuffer);
//  fIsDark = (isDarkBuffer == 1);
//  fIsHVOk = (isHVOkBuffer == 1);
//  fIsDummy= (isDummyBuffer == 1);
//}
