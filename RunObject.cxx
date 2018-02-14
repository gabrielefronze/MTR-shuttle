//
// Created by Gabriele Gaetano Fronzé on 01/02/2018.
//

#include "RunObject.h"

RunObject::RunObject(uint64_t fSOR, uint64_t fEOR, double fAvgHV, double fAvgITot, double fAvgIDark, double fIntCharge,
                     uint64_t fScalBending, uint64_t fScalNotBending, bool fIsDark) : fSOR(fSOR),
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
  sscanf(csvLine.c_str(),"%d;%d;%d;%llu;%llu;%llu;%lf;%lf;%lf;%lf;%lf;%lf;%d",
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

template<typename Type>
std::string getLabel(Type (RunObject::*getter)() const, bool normalizedToArea)
{
  std::string label;

  if(isTimestamp(getter)) label="Timestamp [s]";
  else if(isHV(getter)) label="Voltage [V]";
  else if(isIntCharge(getter)) label=(normalizedToArea)?"Integrated charge [#{mu}C/cm^{2}]":"Integrated charge [#{mu}C]";
  else if(isScaler(getter)) label=(normalizedToArea)?"Hits [Hz/cm^{2}]":"Hits [Hz]";
  else if(isCurrent(getter)) {
    label=(normalizedToArea)?"urrent [#{mu}A/cm^{2}]":"urrent [#{mu}A]";
    if(compareFunctions(getter,&RunObject::getAvgITot)) label="Total c"+label;
    else label="Dark c"+label;
  }

  return label;
}
