//
// Created by Gabriele Gaetano Fronzé on 01/02/2018.
//

#ifndef MTR_SHUTTLE_AMANDACURRENT_H
#define MTR_SHUTTLE_AMANDACURRENT_H

#include "AMANDAData.h"

class AMANDACurrent : AMANDAData
{
  public:
    AMANDACurrent(double fITot, double fIDark, bool fIsDark) = default;

    inline double getITot() const { return fITot; }
    inline void setITot(double fITot) { AMANDACurrent::fITot = fITot; }

    inline double getIDark() const { return fIDark; }
    inline void setIDark(double fIDark) { AMANDACurrent::fIDark = fIDark; }

    inline double getINet() const { return fITot  - fIDark; }

    inline double getIsDark() const { return fIsDark; }

  private:
    double fITot;
    double fIDark;
    bool fIsDark;
};

#endif //MTR_SHUTTLE_AMANDACURRENT_H
