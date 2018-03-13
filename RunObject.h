//
// Created by Gabriele Gaetano Fronzé on 01/02/2018.
//

#ifndef MTR_SHUTTLE_RUNOBJECT_H
#define MTR_SHUTTLE_RUNOBJECT_H


#include <cstdint>
#include <fstream>
#include "Parameters.h"

class RunObject{
  public:
    explicit RunObject(uint64_t SOR=0, uint64_t EOR=0, double avgHV=0., double avgITot=0., double avgIDark=0.,
              double intCharge=0., uint64_t scalBending=0, uint64_t scalNotBending=0,
              bool isDark=false);

    RunObject(std::string csvLine, int &plane, int &side, int &RPC);

    inline uint64_t getRunNumber() const { return fRunNumber; }
    inline void setRunNumber(uint64_t runNumber) { RunObject::fRunNumber = runNumber; }

    inline uint64_t getSOR() const { return fSOR; }
    inline void setSOR(uint64_t SOR) { RunObject::fSOR = SOR; }

    inline uint64_t getEOR() const { return fEOR; }
    inline void setEOR(uint64_t EOR) { RunObject::fEOR = EOR; }

    inline double getAvgHV() const { return fAvgHV; }
    inline void setAvgHV(double avgHV) { RunObject::fAvgHV = avgHV; }

    inline double getAvgITot() const { return fAvgITot; }
    inline void setAvgITot(double avgITot) { RunObject::fAvgITot = avgITot; }

    inline double getAvgIDark() const { return fAvgIDark; }
    inline void setAvgIDark(double avgIDark) { RunObject::fAvgIDark = avgIDark; }

    inline double getAvgINet() const { return (fAvgITot<=fAvgIDark)?(fAvgITot - fAvgIDark):0.; }

    inline double getIntCharge() const { return fIntCharge; }
    inline void setIntCharge(double intCharge) { RunObject::fIntCharge = intCharge; }

    inline double getScalBending() const { return fScalBending; }
    inline void setScalBending(double scalBending) { RunObject::fScalBending = scalBending; }

    inline double getScalNotBending() const { return fScalNotBending; }
    inline void setScalNotBending(double scalNotBending) { RunObject::fScalNotBending = scalNotBending; }

    inline void setfIsDark(bool isDark, double /*second*/=0.) { RunObject::fIsDark = isDark; }
    inline void setfIsHVOk(bool isHVOk, double /*second*/=0.) { RunObject::fIsHVOk = isHVOk; }

    // Conditions
    inline bool isDark(uint64_t /*first*/=0., uint64_t /*second*/=0.) const { return fIsDark; }

    inline bool isHVOk(uint64_t /*first*/=0., uint64_t /*second*/=0.) const { return fIsHVOk; }

    inline bool isAfterRun(uint64_t run=0, uint64_t /*second*/=0.) const { return fRunNumber>run; }
    inline bool isBeforeRun(uint64_t run=UINT64_MAX, uint64_t /*second*/=0.) const { return fRunNumber<run; }
    inline bool isBetweenRuns(uint64_t runMin, uint64_t runMax) const {
      return (isBeforeRun(runMax) && isAfterRun(runMin));
    }

    inline bool isBefore(uint64_t TS=0, uint64_t /*second*/=0.) const { return fEOR<TS; }
    inline bool isAfter(uint64_t TS=UINT64_MAX, uint64_t /*second*/=0.) const { return fSOR>TS; }
    inline bool isBetweenTimestamps(uint64_t TSMin, uint64_t TSMax) const {
      return (isBefore(TSMax) && isAfter(TSMin));
    }

    inline bool isValidForIntCharge(uint64_t /*first*/=0., uint64_t /*second*/=0.) const { return (!fIsDark && fIsHVOk); }
    inline bool isValidForIDark(uint64_t /*first*/=0., uint64_t /*second*/=0.) const { return (fIsDark && fIsHVOk); }

    constexpr bool getTrue(uint64_t /*first*/=0., uint64_t /*second*/=0.) const { return true; }

    friend std::ostream& operator<<(std::ostream& os, const RunObject& obj);

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
    bool fIsHVOk;
    bool fIsDark;
};

inline RunObject operator+ (RunObject result, const RunObject &added){
  result.setAvgIDark(result.getAvgIDark()+added.getAvgIDark());
  result.setAvgITot(result.getAvgITot()+added.getAvgITot());
  result.setAvgHV(result.getAvgHV()+added.getAvgHV());
  result.setIntCharge(result.getIntCharge()+added.getIntCharge());
  result.setScalBending(result.getScalBending()+added.getScalBending());
  result.setScalNotBending(result.getScalNotBending()+added.getScalNotBending());
  return result;
}

inline RunObject operator/ (RunObject result, const double &divider){
  result.setAvgIDark(result.getAvgIDark()/divider);
  result.setAvgITot(result.getAvgITot()/divider);
  result.setAvgHV(result.getAvgHV()/divider);
  result.setIntCharge(result.getIntCharge()/divider);
  result.setScalBending(result.getScalBending()/divider);
  result.setScalNotBending(result.getScalNotBending()/divider);
  return result;
}

std::ostream& operator<<(std::ostream& os, const RunObject& obj){
  return os << obj.getRunNumber() << ";"
            << obj.getSOR() << ";"
            << obj.getEOR() << ";"
            << obj.getAvgHV() << ";"
            << obj.getAvgITot() << ";"
            << obj.getAvgIDark() << ";"
            << obj.getIntCharge() << ";"
            << obj.getScalBending() << ";"
            << obj.getScalNotBending() << ";"
            << (int) obj.isDark() << ";"
            << (int) obj.isHVOk();
}

template<typename Type> inline bool funcCmp(Type(RunObject::*getX)() const, Type(RunObject::*getY)() const){
  return (getX == getY);
};

template<typename XType, typename YType>
inline typename std::enable_if<!(std::is_same<XType,YType>::value),bool>::type
funcCmp(XType(RunObject::*/*getX*/)() const, YType(RunObject::*/*getY*/)() const){
  return false;
};

template<typename Type> inline bool isTimestamp(Type(RunObject::*getter)() const){
  return (funcCmp(getter, &RunObject::getEOR) || funcCmp(getter, &RunObject::getSOR));
}

template<typename Type> inline bool isCurrent(Type(RunObject::*getter)() const){
  return (funcCmp(getter, &RunObject::getAvgIDark) || funcCmp(getter, &RunObject::getAvgITot));
}

template<typename Type> inline bool isScaler(Type(RunObject::*getter)() const){
  return (funcCmp(getter, &RunObject::getScalBending) || funcCmp(getter, &RunObject::getScalNotBending));
}

template<typename Type> inline bool isHV(Type(RunObject::*getter)() const){
  return funcCmp(getter, &RunObject::getAvgHV);
}

template<typename Type> inline bool isIntCharge(Type(RunObject::*getter)() const){
  return funcCmp(getter, &RunObject::getIntCharge);
}

template<typename Type> std::string getLabel(Type(RunObject::*getter)() const, bool normalizedToArea);


#endif //MTR_SHUTTLE_RUNOBJECT_H