//
// Created by Gabriele Gaetano Fronzé on 22/05/2018.
//

#include "AMANDAVoltage.h"

AMANDAVoltage::AMANDAVoltage(uint64_t timestamp, double HV) : fHV(HV)
{
  this->setTimeStamp(timestamp);
}

AMANDAVoltage::AMANDAVoltage(std::string csvLine, int &plane, int &side, int &RPC)
{
  int bufferIsDark = 0;
  sscanf(csvLine.c_str(),"%d;%d;%d;%llu;%lf",
         &plane,
         &side,
         &RPC,
         &fTimeStamp,
         &fHV);
}