//
// Created by Gabriele Gaetano Fronzé on 01/02/2018.
//

#include "RunObject.h"

RunObject::RunObject(uint64_t SOR, uint64_t EOR, double avgHV, double avgITot, double avgIDark, double intCharge,
                     uint64_t scalBending, uint64_t scalNotBending, bool isDark) : fSOR(SOR),
                                                                              fEOR(EOR),
                                                                              fAvgHV(avgHV),
                                                                              fAvgITot(avgITot),
                                                                              fAvgIDark(avgIDark),
                                                                              fIntCharge(intCharge),
                                                                              fScalBending(scalBending),
                                                                              fScalNotBending(scalNotBending),
                                                                              fIsDark(isDark)
{}

RunObject::RunObject(std::string csvLine, int &plane, int &side, int &RPC)
{
  int isDarkBuffer = 0;
  int isHVOkBuffer = 0;
  sscanf(csvLine.c_str(),"%d;%d;%d;%llu;%llu;%llu;%lf;%lf;%lf;%lf;%lf;%lf;%d;%d",
         &plane,
         &side,
         &RPC,
         &fRunNumber,
         &fSOR,
         &fEOR,
         &fAvgHV,
         &fAvgITot,
         &fAvgIDark,
         &fIntCharge,
         &fScalBending,
         &fScalNotBending,
         &isDarkBuffer,
         &isHVOkBuffer);
  fIsDark = (isDarkBuffer == 1);
  fIsHVOk = (isHVOkBuffer == 1);
}
