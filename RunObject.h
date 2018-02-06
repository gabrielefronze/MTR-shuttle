//
// Created by Gabriele Gaetano Fronzé on 01/02/2018.
//

#ifndef MTR_SHUTTLE_RUNOBJECT_H
#define MTR_SHUTTLE_RUNOBJECT_H

#include <cstdint>
#include <fstream>

class RunObject{
  public:
    RunObject(uint64_t fSOR=0, uint64_t fEOR=0, double fAvgHV=0., double fAvgITot=0., double fAvgIDark=0.,
              double fIntCharge=0., uint64_t fScalBending=0, uint64_t fScalNotBending=0,
              bool fIsDark=false);

    RunObject(std::string csvLine, int &plane, int &side, int &RPC);

    inline uint64_t getRunNumber() const { return fRunNumber; }
    inline void setRunNumber(uint64_t fRunNumber) { RunObject::fRunNumber = fRunNumber; }

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

    inline double getScalBending() const { return fScalBending; }
    inline void setScalBending(double fScalBending) { RunObject::fScalBending = fScalBending; }

    inline double getScalNotBending() const { return fScalNotBending; }
    inline void setScalNotBending(double fScalNotBending) { RunObject::fScalNotBending = fScalNotBending; }

    inline bool isDark() const { return fIsDark; }
    inline void setfIsDark(bool fIsDark) { RunObject::fIsDark = fIsDark; }

    friend std::ostream& operator<<(std::ostream& os, const RunObject& dt);

  private:
    uint64_t fRunNumber;
    uint64_t fSOR;
    uint64_t fEOR;
    double fAvgHV;
    double fAvgITot;
    double fAvgIDark;
    double fIntCharge;
    double fScalBending;
    double fScalNotBending;
    bool fIsDark;
};

std::ostream& operator<<(std::ostream& os, const RunObject& obj){
  return os << obj.getRunNumber() << "\t" << obj.getSOR() << "\t" << obj.getEOR() << "\t" << obj.getAvgHV()
            << "\t" << obj.getAvgITot() << "\t" << obj.getAvgIDark() << "\t" << obj.getIntCharge()
            << "\t" << obj.getScalBending() << "\t" << obj.getScalNotBending() << "\t" << (int) obj.isDark();
}

#endif //MTR_SHUTTLE_RUNOBJECT_H