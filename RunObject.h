//
// Created by Gabriele Gaetano Fronzé on 01/02/2018.
//

#ifndef MTR_SHUTTLE_RUNOBJECT_H
#define MTR_SHUTTLE_RUNOBJECT_H


#include <stdint.h>
#include <fstream>
#include "Parameters.h"

class RunObject{
  public:
  explicit RunObject(uint64_t SOR=0, uint64_t EOR=0, double avgHV=0., double avgITot=0., double avgIDark=0.,
                     double intCharge=0., uint64_t scalBending=0, uint64_t scalNotBending=0,
                     bool isDark=false, bool isDummy=false);

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

  inline bool isDark() const { return fIsDark; }
  inline void setfIsDark(bool isDark) { RunObject::fIsDark = isDark; }

  inline bool isHVOk() const { return fIsHVOk; }
  inline void setfIsHVOk(bool isHVOk) { RunObject::fIsHVOk = isHVOk; }

  inline bool isDummy() const { return fIsDummy; }
  inline void setfIsDummy(bool isDummy) { RunObject::fIsDummy = isDummy; }

  inline bool isAfterRun(uint64_t run=0) const { return fRunNumber>run; }
  inline bool isBeforeRun(uint64_t run=UINT64_MAX) const { return fRunNumber<run; }
  inline bool isBetweenRuns(uint64_t runMin, uint64_t runMax) const {
    return (isBeforeRun(runMax) && isAfterRun(runMin));
  }

  inline bool isBefore(uint64_t TS=0) const { return fEOR<TS; }
  inline bool isAfter(uint64_t TS=UINT64_MAX) const { return fSOR>TS; }
  inline bool isBetweenTimestamps(uint64_t TSMin, uint64_t TSMax) const {
    return (isBefore(TSMax) && isAfter(TSMin));
  }

  inline bool isValidForIntCharge() const { return fIsHVOk; }
  inline bool isValidForIDark() const { return (fIsDark && fIsHVOk); }

  inline void reset(){
    this->setAvgIDark(0.);
    this->setAvgITot(0.);
    this->setAvgHV(0.);
    this->setIntCharge(0.);
    this->setScalBending(0);
    this->setScalNotBending(0);
  }

//    template<class ...Args> constexpr bool getTrue(Args... /*arg*/) const { return true; }

  friend std::ostream& operator<<(std::ostream& os, const RunObject& obj);
  friend RunObject operator+(RunObject result, const RunObject &added);
  friend RunObject operator/(RunObject result, const double &divider);

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
  bool fIsDummy;
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
            << (int) obj.isHVOk() << ";"
            << (int) obj.isDummy();
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

template<typename Type> inline std::string generateLabel(Type(RunObject::*getter)() const, bool normalizedToArea){
  std::string label="";

  if(isTimestamp(getter)) label="Timestamp [s]";
  else if(isHV(getter)) label="Voltage [V]";
  else if(isIntCharge(getter)) label=(normalizedToArea)?"Integrated charge [#muC/cm^{2}]":"Integrated charge [#muC]";
  else if(isScaler(getter)) label=(normalizedToArea)?"Hits [Hz/cm^{2}]":"Hits [Hz]";
  else if(isCurrent(getter)) {
    label=(normalizedToArea)?"urrent [#muA/cm^{2}]":"urrent [#muA]";
    if(funcCmp(getter, &RunObject::getAvgITot)) label="Total c"+label;
    else if(funcCmp(getter, &RunObject::getAvgINet)) label="Net c"+label;
    else label="Dark c"+label;
  }

  return label;
}

template<typename Type> inline std::string generateTitle(Type(RunObject::*getter)() const){
  std::string title="";

  if(isHV(getter)) title="HV";
  else if(isIntCharge(getter)) title="Integrated charge";
  else if(isScaler(getter)) title="Hits";
  else if(isCurrent(getter)) {
    title="urrent";
    if(funcCmp(getter, &RunObject::getAvgITot)) title="Total c"+title;
    else if(funcCmp(getter, &RunObject::getAvgINet)) title="Net c"+title;
    else title="Dark c"+title;
  }

  return title;
}

#endif //MTR_SHUTTLE_RUNOBJECT_H