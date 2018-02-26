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

template<typename Type>
std::string getLabel(Type (RunObject::*getter)() const, bool normalizedToArea)
{
  std::string label;

  if(isTimestamp(getter)) label="Timestamp [s]";
  else if(isHV(getter)) label="Voltage [V]";
  else if(isIntCharge(getter)) label=(normalizedToArea)?"Integrated charge [#muC/cm^{2}]":"Integrated charge [#muC]";
  else if(isScaler(getter)) label=(normalizedToArea)?"Hits [Hz/cm^{2}]":"Hits [Hz]";
  else if(isCurrent(getter)) {
    label=(normalizedToArea)?"urrent [#muA/cm^{2}]":"urrent [#muA]";
    if(compareFunctions(getter,&RunObject::getAvgITot)) label="Total c"+label;
    else label="Dark c"+label;
  }

  return label;
}
