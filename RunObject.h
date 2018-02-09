//
// Created by Gabriele Gaetano Fronzé on 01/02/2018.
//

#ifndef MTR_SHUTTLE_RUNOBJECT_H
#define MTR_SHUTTLE_RUNOBJECT_H


#include <cstdint>
#include <fstream>

class RunObject{
  public:
    explicit RunObject(uint64_t fSOR=0, uint64_t fEOR=0, double fAvgHV=0., double fAvgITot=0., double fAvgIDark=0.,
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

    inline bool getTrue() const { return true; }

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
  return os << obj.getRunNumber() << ";" << obj.getSOR() << ";" << obj.getEOR() << ";" << obj.getAvgHV()
            << ";" << obj.getAvgITot() << ";" << obj.getAvgIDark() << ";" << obj.getIntCharge()
            << ";" << obj.getScalBending() << ";" << obj.getScalNotBending() << ";" << (int) obj.isDark();
}

template<typename Type> inline bool compareFunctions(Type(RunObject::*getX)() const, Type(RunObject::*getY)() const){
  return (getX == getY);
};

template<typename XType, typename YType>
inline typename std::enable_if<!(std::is_same<XType,YType>::value),bool>::type
compareFunctions(XType(RunObject::*getX)() const, YType(RunObject::*getY)() const){
  return false;
};

template<typename Type> inline bool isTimestamp(Type(RunObject::*getter)() const){
  return (compareFunctions(getter,&RunObject::getEOR) || compareFunctions(getter,&RunObject::getSOR));
}

template<typename Type> inline bool isCurrent(Type(RunObject::*getter)() const){
  return (compareFunctions(getter,&RunObject::getAvgIDark) || compareFunctions(getter,&RunObject::getAvgITot));
}

template<typename Type> inline bool isScaler(Type(RunObject::*getter)() const){
  return (compareFunctions(getter,&RunObject::getScalBending) || compareFunctions(getter,&RunObject::getScalNotBending));
}

template<typename Type> inline bool isHV(Type(RunObject::*getter)() const){
  return compareFunctions(getter,&RunObject::getAvgHV);
}

template<typename Type> inline bool isIntCharge(Type(RunObject::*getter)() const){
  return compareFunctions(getter,&RunObject::getIntCharge);
}

template<typename Type> std::string getLabel(Type(RunObject::*getter)() const, bool normalizedToArea);


#endif //MTR_SHUTTLE_RUNOBJECT_H