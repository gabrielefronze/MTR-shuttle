//
// Created by Gabriele Gaetano Fronzé on 01/02/2018.
//

#ifndef MTR_SHUTTLE_AMANDACURRENT_H
#define MTR_SHUTTLE_AMANDACURRENT_H

#include "AMANDAData.h"
#include <fstream>

class AMANDACurrent : public AMANDAData
{
  public:
  explicit AMANDACurrent(uint64_t timestamp=0ULL, double iTot = 0., double iDark = 0., bool isDarkCurrent = false, bool isHVOk = false);
  AMANDACurrent(std::string csvLine, int &plane, int &side, int &RPC);

  inline double getITot() const { return fITot; }
  inline void setITot(double iTot) { fITot = iTot; }

  inline double getIDark() const { return (fIsDarkCurrent)?fITot:fIDark; }
  inline void setIDark(double iDark) { fIDark = iDark; }

  inline double getINet() const {
    auto iNet = fITot  - this->getIDark();
    return (iNet>0.)?iNet:0.;
  }

  inline bool isDark() const { return fIsDarkCurrent; }
  inline void setIsDark(bool isDark) { fIsDarkCurrent = isDark; }

  inline bool isHvOk() const { return fIsHVOk; }
  inline bool hasBeenFlagged() const { return fFlagged; }
  inline void setIsHvOk(bool isHvOk) { fIsHVOk = isHvOk; fFlagged=true; }

  friend std::ostream& operator<<(std::ostream& os, const AMANDACurrent& obj);

  private:
  double fITot;
  double fIDark;
  bool fIsDarkCurrent;
  bool fIsHVOk;
  bool fFlagged;
};

std::ostream& operator<<(std::ostream& os, const AMANDACurrent& obj){
  return os << obj.getTimeStamp() << ";"
            << obj.getITot() << ";"
            << obj.getIDark() << ";"
            << obj.isDark() << ";"
            << obj.isHvOk() << ";"
            << obj.hasBeenFlagged();
}

#endif //MTR_SHUTTLE_AMANDACURRENT_H
