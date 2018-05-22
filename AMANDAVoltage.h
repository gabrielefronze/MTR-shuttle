//
// Created by Gabriele Gaetano Fronzé on 01/02/2018.
//

#ifndef MTR_SHUTTLE_AMANDAVOLTAGE_H
#define MTR_SHUTTLE_AMANDAVOLTAGE_H

#include "AMANDAData.h"
#include <fstream>

class AMANDAVoltage : public AMANDAData
{
  public:
  explicit AMANDAVoltage(uint64_t timestamp=0ULL, double HV = 0);
  AMANDAVoltage(std::string csvLine, int &plane, int &side, int &RPC);

  inline double getHV() const { return fHV; }
  inline void setHV(double HV) { fHV = HV; }

  friend std::ostream& operator<<(std::ostream& os, const AMANDAVoltage& obj);

  private:
  double fHV;
};

std::ostream& operator<<(std::ostream& os, const AMANDAVoltage& obj){
  return os << obj.getHV();
}

#endif //MTR_SHUTTLE_AMANDAVOLTAGE_H
