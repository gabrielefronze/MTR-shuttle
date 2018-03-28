//
// Created by Gabriele Gaetano Fronzé on 01/02/2018.
//

#include "AMANDACurrent.h"

AMANDACurrent::AMANDACurrent(uint64_t timestamp, double iTot, double iDark, bool isDarkCurrent) : fITot(iTot),
                                                                                                  fIDark(iDark),
                                                                                                  fIsDarkCurrent(isDarkCurrent)
{
  this->setTimeStamp(timestamp);
}

AMANDACurrent::AMANDACurrent(std::string csvLine, int &plane, int &side, int &RPC)
{
  int bufferIsDark = 0;
  sscanf(csvLine.c_str(),"%d;%d;%d;%llu;%lf;%lf;%d",
         &plane,
         &side,
         &RPC,
         &fTimeStamp,
         &fITot,
         &fIDark,
         &bufferIsDark);
  fIsDarkCurrent = (bufferIsDark==1);


}
