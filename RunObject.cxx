//
// Created by Gabriele Gaetano Fronzé on 01/02/2018.
//

#include "RunObject.h"

RunObject::RunObject(uint64_t fSOR, uint64_t fEOR, double fAvgHV, double fAvgITot, double fAvgIDark, double fIntCharge,
                     uint64_t fScalBending, uint64_t fScalNotBending, bool fIsDark=false) : fSOR(fSOR),
                                                                              fEOR(fEOR),
                                                                              fAvgHV(fAvgHV),
                                                                              fAvgITot(fAvgITot),
                                                                              fAvgIDark(fAvgIDark),
                                                                              fIntCharge(fIntCharge),
                                                                              fScalBending(fScalBending),
                                                                              fScalNotBending(fScalNotBending),
                                                                              fIsDark(fIsDark)
{}

RunObject::RunObject(std::string csvLine, int &plane, int &side, int &RPC)
{
  int ifDarkBuffer = 0;
  sscanf(csvLine.c_str(),"%d,%d,%d,%llu,%llu,%llu,%lf,%lf,%lf,%lf,%lf,%lf,%d",
         &plane,
         &side,
         &RPC,
         &fRunNumber,
         &fSOR,
         &fSOR,
         &fAvgHV,
         &fAvgITot,
         &fAvgIDark,
         &fIntCharge,
         &fScalBending,
         &fScalNotBending,
         &ifDarkBuffer);
  fIsDark = (ifDarkBuffer == 1);
}
