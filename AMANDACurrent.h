//
// Created by Gabriele Gaetano Fronzé on 01/02/2018.
//

#ifndef MTR_SHUTTLE_AMANDACURRENT_H
#define MTR_SHUTTLE_AMANDACURRENT_H

#include "AMANDAData.h"

class AMANDACurrent : AMANDAData
{
  public:
    AMANDACurrent(double fITot = 0., double fIDark = 0., bool fIsDarkCurrent = false);

    inline double getITot() const { return fITot; }
    inline void setITot(double fITot) { AMANDACurrent::fITot = fITot; }

    inline double getIDark() const { return fIDark; }
    inline void setIDark(double fIDark) { AMANDACurrent::fIDark = fIDark; }

    inline double getINet() const {
      auto iNet = fITot  - fIDark;
      return (iNet>0.)?iNet:0.;
    }

    inline bool isDark() const { return fIsDarkCurrent; }
    inline void setIsDark(bool fIsDark) { AMANDACurrent::fIsDarkCurrent = fIsDark; }

  private:
    double fITot;
    double fIDark;
    bool fIsDarkCurrent;
};

#endif //MTR_SHUTTLE_AMANDACURRENT_H
