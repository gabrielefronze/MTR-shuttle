//
// Created by Gabriele Gaetano Fronzé on 01/02/2018.
//

#include "AMANDACurrent.h"

AMANDACurrent::AMANDACurrent(uint64_t timestamp, double iTot, double iDark, bool isDarkCurrent, bool isHvOk) : fITot(iTot),
                                                                                                               fIDark(iDark),
                                                                                                               fIsDarkCurrent(isDarkCurrent),
                                                                                                               fIsHVOk(isHvOk),
                                                                                                               fFlagged(false)
{
  this->setTimeStamp(timestamp);
}

AMANDACurrent::AMANDACurrent(std::string csvLine, int &plane, int &side, int &RPC)
{
  int bufferIsDark = 0;
  int bufferIsHvOk = 0;
  int bufferFlagged = 0;
  sscanf(csvLine.c_str(),"%d;%d;%d;%llu;%lf;%lf;%d;%d;%d",
         &plane,
         &side,
         &RPC,
         &fTimeStamp,
         &fITot,
         &fIDark,
         &bufferIsDark,
         &bufferIsHvOk,
         &bufferFlagged

  );
  fIsDarkCurrent = (bufferIsDark==1);
  fIsHVOk = (bufferIsHvOk==1);
  fFlagged = (bufferFlagged==1);
}
