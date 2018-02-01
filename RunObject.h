//
// Created by Gabriele Gaetano Fronzé on 01/02/2018.
//

#include <cstdint>

class RunObject{
  public:
    RunObject(uint64_t fSOR=0, uint64_t fEOR=0, double fAvgHV=0., double fAvgITot=0., double fAvgIDark=0.,
              double fIntCharge=0., uint64_t fScalersBending=0, uint64_t fScalersNotBending=0) = default;

    inline uint64_t getSOR() const { return fSOR; }

    inline void setSOR(uint64_t fSOR) { RunObject::fSOR = fSOR; }

    inline uint64_t getEOR() const { return fEOR; }

    inline void setEOR(uint64_t fEOR) { RunObject::fEOR = fEOR; }

    inline double getAvgHV() const { return fAvgHV; }

    inline void setAvgHV(double fAvgHV) { RunObject::fAvgHV = fAvgHV; }

    inline double getAvgITot() const { return fAvgITot; }

    inline void setAvgITot(double fAvgITot) { RunObject::fAvgITot = fAvgITot; }

    inline double getAvgIDark() const { return fAvgIDark; }

    inline void setAvgIDark(double fAvgIDark) { RunObject::fAvgIDark = fAvgIDark; }

    inline double getIntCharge() const { return fIntCharge; }

    inline void setIntCharge(double fIntCharge) { RunObject::fIntCharge = fIntCharge; }

    inline uint64_t getScalersBending() const { return fScalersBending; }

    inline void setScalersBending(uint64_t fScalersBending) { RunObject::fScalersBending = fScalersBending; }

    inline uint64_t getScalersNotBending() const { return fScalersNotBending; }

    inline void setScalersNotBending(uint64_t fScalersNotBending) { RunObject::fScalersNotBending = fScalersNotBending; }

  private:
    uint64_t fSOR;
    uint64_t fEOR;
    double fAvgHV;
    double fAvgITot;
    double fAvgIDark;
    double fIntCharge;
    uint64_t fScalersBending;
    uint64_t fScalersNotBending;
};