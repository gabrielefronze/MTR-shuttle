//
// Created by Gabriele Gaetano Fronzé on 01/02/2018.
//

#ifndef MTR_SHUTTLE_RUNOBJECT_H
#define MTR_SHUTTLE_RUNOBJECT_H


#include <stdint.h>
#include <fstream>
#include "Parameters.h"

const auto totRPCN = MTRPlanes::kNPlanes*MTRSides::kNSides*MTRRPCs::kNRPCs;

class RunObject{
  public:
  explicit RunObject(uint64_t SOR=0, uint64_t EOR=0, double avgHV=0., double avgITot=0., double avgIDark=0.,
                     double intCharge=0., uint64_t scalBending=0, uint64_t scalNotBending=0,
                     bool isDark=false, bool isDummy=false);

  RunObject(uint64_t SOR, uint64_t EOR, double avgHV[totRPCN], double avgITot[totRPCN], double avgIDark[totRPCN], double intCharge[totRPCN],
            uint64_t scalBending[totRPCN], uint64_t scalNotBending[totRPCN], bool isDark, bool isDummy);

//  RunObject(std::string csvLine, int &plane, int &side, int &RPC);

  inline uint64_t getRunNumber(MTRPlanes /*p*/=kNPlanes, MTRSides /*s*/=kNSides, MTRRPCs /*r*/=kNRPCs) const { return fRunNumber; }
  inline void setRunNumber(uint64_t runNumber) { fRunNumber = runNumber; }

  inline uint64_t getSOR(MTRPlanes /*p*/=kNPlanes, MTRSides /*s*/=kNSides, MTRRPCs /*r*/=kNRPCs) const { return fSOR; }
  inline void setSOR(uint64_t SOR) { fSOR = SOR; }

  inline uint64_t getEOR(MTRPlanes /*p*/=kNPlanes, MTRSides /*s*/=kNSides, MTRRPCs /*r*/=kNRPCs) const { return fEOR; }
  inline void setEOR(uint64_t EOR) { fEOR = EOR; }

  inline bool isDark() const { return fIsDark; }
  inline void setIsDark(bool isDark) { fIsDark = isDark; }

  inline bool isDummy() const { return fIsDummy; }
  inline void setIsDummy(bool isDummy) { fIsDummy = isDummy; }

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

  size_t index(MTRPlanes p, MTRSides s, MTRRPCs r) const {
    return p*(MTRSides::kNSides)*(MTRRPCs::kNRPCs)+s*(MTRRPCs::kNRPCs)+r;
  }

  inline double getAvgHV(MTRPlanes p=kNPlanes, MTRSides s=kNSides, MTRRPCs r=kNRPCs) const { return getValueSmart(fAvgHV,p,s,r); }
  inline void setAvgHV(double avgHV, MTRPlanes p=kNPlanes, MTRSides s=kNSides, MTRRPCs r=kNRPCs) { setValueSmart(avgHV,fAvgHV,p,s,r); }

  inline double getAvgITot(MTRPlanes p=kNPlanes, MTRSides s=kNSides, MTRRPCs r=kNRPCs) const { return getValueSmart(fAvgITot,p,s,r); }
  inline void setAvgITot(double avgHV, MTRPlanes p=kNPlanes, MTRSides s=kNSides, MTRRPCs r=kNRPCs) { setValueSmart(avgHV,fAvgITot,p,s,r); }

  inline double getAvgIDark(MTRPlanes p=kNPlanes, MTRSides s=kNSides, MTRRPCs r=kNRPCs) const { return getValueSmart(fAvgIDark,p,s,r); }
  inline void setAvgIDark(double avgHV, MTRPlanes p=kNPlanes, MTRSides s=kNSides, MTRRPCs r=kNRPCs) { setValueSmart(avgHV,fAvgIDark,p,s,r); }

  inline double getAvgINet(MTRPlanes p=kNPlanes, MTRSides s=kNSides, MTRRPCs r=kNRPCs) const { return getAvgITot(p,s,r)-getAvgIDark(p,s,r); }

  inline double getIntCharge(MTRPlanes p=kNPlanes, MTRSides s=kNSides, MTRRPCs r=kNRPCs) const { return getValueSmart(fIntCharge,p,s,r); }
  inline void setIntCharge(double avgHV, MTRPlanes p=kNPlanes, MTRSides s=kNSides, MTRRPCs r=kNRPCs) { setValueSmart(avgHV,fIntCharge,p,s,r); }

  inline double getScalBending(MTRPlanes p=kNPlanes, MTRSides s=kNSides, MTRRPCs r=kNRPCs) const { return getValueSmart(fScalBending,p,s,r); }
  inline void setScalBending(uint64_t avgHV, MTRPlanes p=kNPlanes, MTRSides s=kNSides, MTRRPCs r=kNRPCs) { setValueSmart(avgHV,fScalBending,p,s,r); }

  inline double getScalNotBending(MTRPlanes p=kNPlanes, MTRSides s=kNSides, MTRRPCs r=kNRPCs) const { return getValueSmart(fScalNotBending,p,s,r); }
  inline void setScalNotBending(uint64_t avgHV, MTRPlanes p=kNPlanes, MTRSides s=kNSides, MTRRPCs r=kNRPCs) { setValueSmart(avgHV,fScalNotBending,p,s,r); }

  inline bool isHVOk(MTRPlanes p, MTRSides s, MTRRPCs r) const { return getValue(p,s,r,fIsHVOk); }
  inline void setIsHVOk(bool avgHV, MTRPlanes p=kNPlanes, MTRSides s=kNSides, MTRRPCs r=kNRPCs) { setValueSmart(avgHV,fIsHVOk,p,s,r); }

  inline bool isValidForIntCharge(MTRPlanes p, MTRSides s, MTRRPCs r) const { return isHVOk(p,s,r); }
  inline bool isValidForIDark(MTRPlanes p, MTRSides s, MTRRPCs r) const { return (fIsDark && isHVOk(p,s,r)); }

  inline void reset(){
    this->setAvgIDark(0.);
    this->setAvgITot(0.);
    this->setAvgHV(0.);
    this->setIntCharge(0.);
    this->setScalBending(0);
    this->setScalNotBending(0);
  }

  friend std::ostream& operator<<(std::ostream& os, const RunObject& obj);
  friend RunObject operator+(RunObject result, const RunObject &added);
  friend RunObject operator/(RunObject result, const double &divider);

  private:
  uint64_t fRunNumber;
  uint64_t fSOR;
  uint64_t fEOR;
  double fAvgHV[totRPCN];
  double fAvgITot[totRPCN];
  double fAvgIDark[totRPCN];
  double fIntCharge[totRPCN];
  uint64_t fScalBending[totRPCN];
  uint64_t fScalNotBending[totRPCN];
  bool fIsHVOk[totRPCN];
  bool fIsDark;
  bool fIsDummy;

  // Template getters
  template<class returnT> returnT getValue(MTRPlanes p, MTRSides s, MTRRPCs r, returnT data[totRPCN]) const { return data[index(p,s,r)]; }
  template<class returnT> returnT getValue(MTRPlanes p, MTRSides s, returnT data[totRPCN]) const {
    auto cumulus = (returnT)0;
    uint8_t counter = 0;
    for (int iRPC = 0; iRPC < MTRRPCs::kNRPCs; ++iRPC) {
      cumulus+=getValue(p,s,(MTRRPCs)iRPC,data);
      counter++;
    }
    return (returnT)((double)cumulus/(double)counter);
  }
  template<class returnT> returnT getValue(MTRPlanes p, returnT data[totRPCN]) const {
    auto cumulus = (returnT)0;
    uint8_t counter = 0;
    for (int iSide = 0; iSide < MTRSides::kNSides; ++iSide) {
      cumulus+=getValue(p,(MTRSides)iSide,data);
      counter++;
    }
    return (returnT)((double)cumulus/(double)counter);
  }
  template<class returnT> returnT getValue(returnT data[totRPCN]) const {
    auto cumulus = (returnT)0;
    uint8_t counter = 0;
    for (int iPlane = 0; iPlane < MTRPlanes::kNPlanes; ++iPlane) {
      cumulus+=getValue((MTRPlanes)iPlane,data);
      counter++;
    }
    return (returnT)((double)cumulus/(double)counter);
  }
  template<class returnT> returnT getValueSmart(returnT data[totRPCN], MTRPlanes p, MTRSides s, MTRRPCs r) const {
    if(p==kNPlanes) return getValue<returnT>(data);
    if(s==kNSides) return getValue<returnT>(p,data);
    if(r==kNRPCs) return getValue<returnT>(p,s,data);
    return getValue<returnT>(p,s,r,data);
  }

  // Template setters
  template<class dataT> void setValue(dataT value, MTRPlanes p, MTRSides s, MTRRPCs r, dataT data[totRPCN]){ data[index(p,s,r)] = value; }
  template<class dataT> void setValue(dataT value, MTRPlanes p, MTRSides s, dataT data[totRPCN]){
    for (int iRPC = 0; iRPC < MTRRPCs::kNRPCs; ++iRPC) {
      setValue(value,p,s,(MTRRPCs)iRPC,data);
    }
  }
  template<class dataT> void setValue(dataT value, MTRPlanes p, dataT data[totRPCN]){
    for (int iSide = 0; iSide < MTRSides::kNSides; ++iSide) {
      setValue(value,p,(MTRSides)iSide,data);
    }
  }
  template<class dataT> void setValue(dataT value, dataT data[totRPCN]){
    for (int iPlane = 0; iPlane < MTRPlanes::kNPlanes; ++iPlane) {
      setValue(value,(MTRPlanes)iPlane,data);
    }
  }
  template<class dataT> void setValueSmart(dataT value, dataT data[totRPCN], MTRPlanes p, MTRSides s, MTRRPCs r){
    if(p==kNPlanes) return setValue<dataT>(value,data);
    if(s==kNSides) return setValue<dataT>(value,p,data);
    if(r==kNRPCs) return setValue<dataT>(value,p,s,data);
    return setValue<dataT>(value,p,s,r,data);
  }
};

inline RunObject operator+ (RunObject result, const RunObject &added){
  for (int iPlane = 0; iPlane < MTRPlanes::kNPlanes; ++iPlane) {
    for (int iSide = 0; iSide < MTRSides::kNSides; ++iSide) {
      for (int iRPC = 0; iRPC < MTRRPCs::kNRPCs; ++iRPC) {
        auto p = (MTRPlanes)iPlane;
        auto s = (MTRSides)iSide;
        auto r = (MTRRPCs)iRPC;
        result.setAvgIDark(result.getAvgIDark(p,s,r)+added.getAvgIDark(p,s,r),p,s,r);
        result.setAvgITot(result.getAvgITot(p,s,r)+added.getAvgITot(p,s,r),p,s,r);
        result.setAvgHV(result.getAvgHV(p,s,r)+added.getAvgHV(p,s,r),p,s,r);
        result.setIntCharge(result.getIntCharge(p,s,r)+added.getIntCharge(p,s,r),p,s,r);
        result.setScalBending(result.getScalBending(p,s,r)+added.getScalBending(p,s,r),p,s,r);
        result.setScalNotBending(result.getScalNotBending(p,s,r)+added.getScalNotBending(p,s,r),p,s,r);
      }
    }
  }
  return result;
}

inline RunObject operator/ (RunObject result, const double &divider){
  for (int iPlane = 0; iPlane < MTRPlanes::kNPlanes; ++iPlane) {
    for (int iSide = 0; iSide < MTRSides::kNSides; ++iSide) {
      for (int iRPC = 0; iRPC < MTRRPCs::kNRPCs; ++iRPC) {
        auto p = (MTRPlanes) iPlane;
        auto s = (MTRSides) iSide;
        auto r = (MTRRPCs) iRPC;
        result.setAvgIDark(result.getAvgIDark(p,s,r) / divider);
        result.setAvgITot(result.getAvgITot(p,s,r) / divider);
        result.setAvgHV(result.getAvgHV(p,s,r) / divider);
        result.setIntCharge(result.getIntCharge(p,s,r) / divider);
        result.setScalBending((uint64_t)(result.getScalBending(p,s,r) / divider));
        result.setScalNotBending((uint64_t)(result.getScalNotBending(p,s,r) / divider));
      }
    }
  }
  return result;
}

std::ostream& operator<<(std::ostream& os, const RunObject& obj){
  return os << obj.getRunNumber() << ";"
            << obj.getSOR() << ";"
            << obj.getEOR() << ";"
            << (int) obj.isDark() << ";"
            << (int) obj.isDummy() << ";"
            << obj.getAvgHV() << ";"
//            << (int) obj.isHVOk() << ";"
            << obj.getAvgITot() << ";"
            << obj.getAvgIDark() << ";"
            << obj.getIntCharge() << ";"
            << obj.getScalBending() << ";"
            << obj.getScalNotBending();
}

template<typename Type> inline bool funcCmp(Type(RunObject::*getX)(MTRPlanes px, MTRSides sx, MTRRPCs rx) const, Type(RunObject::*getY)(MTRPlanes py, MTRSides sy, MTRRPCs ry) const){
  return (getX == getY);
};

//template<typename Type, typename... ArgsX, typename... ArgsY>
//inline typename std::enable_if<!(std::is_same<ArgsX,ArgsY>::value),bool>::type
//funcCmp(Type(RunObject::*getX)(ArgsX ...argsX) const, Type(RunObject::*getY)(ArgsY ...argsY) const){
//  return false;
//};

template<typename XType, typename YType>
inline typename std::enable_if<!(std::is_same<XType,YType>::value),bool>::type
funcCmp(XType(RunObject::*/*getX*/)(MTRPlanes px, MTRSides sx, MTRRPCs rx) const, YType(RunObject::*/*getY*/)(MTRPlanes py, MTRSides sy, MTRRPCs ry) const){
  return false;
};

template<typename Type> inline bool isTimestamp(Type(RunObject::*getter)(MTRPlanes p, MTRSides s, MTRRPCs r) const){
  return (funcCmp(getter, &RunObject::getEOR) || funcCmp(getter, &RunObject::getSOR));
}

template<typename Type> inline bool isCurrent(Type(RunObject::*getter)(MTRPlanes p, MTRSides s, MTRRPCs r) const){
  return (funcCmp(getter, &RunObject::getAvgIDark) || funcCmp(getter, &RunObject::getAvgITot));
}

template<typename Type> inline bool isScaler(Type(RunObject::*getter)(MTRPlanes p, MTRSides s, MTRRPCs r) const){
  return (funcCmp(getter, &RunObject::getScalBending) || funcCmp(getter, &RunObject::getScalNotBending));
}

template<typename Type> inline bool isHV(Type(RunObject::*getter)(MTRPlanes p, MTRSides s, MTRRPCs r) const){
  return funcCmp(getter, &RunObject::getAvgHV);
}

template<typename Type> inline bool isIntCharge(Type(RunObject::*getter)(MTRPlanes p, MTRSides s, MTRRPCs r) const){
  return funcCmp(getter, &RunObject::getIntCharge);
}

template<typename Type> inline std::string generateLabel(Type(RunObject::*getter)(MTRPlanes p, MTRSides s, MTRRPCs r) const, bool normalizedToArea){
  std::string label;

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

template<typename Type> inline std::string generateTitle(Type(RunObject::*getter)(MTRPlanes p, MTRSides s, MTRRPCs r) const){
  std::string title;

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