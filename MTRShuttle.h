//
// Created by Gabriele Gaetano Fronzé on 01/02/2018.
//

#ifndef MTR_SHUTTLE_MTRSHUTTLE_H
#define MTR_SHUTTLE_MTRSHUTTLE_H

#include <vector>
#include <string>
#include <TGraph.h>
#include <TMultiGraph.h>
#include "RunObject.h"
#include "AMANDACurrent.h"
#include "Parameters.h"

template<class ...Args> struct PlotCondition{
    bool (RunObject::*fCondition)(Args...);
    Args... fArgs;
    bool fNegate;
};

typedef std::function<bool(RunObject const *)> cond_type;
typedef std::vector<cond_type> cond_vector;
template<class ...Args> static auto binder(bool (RunObject::*condition)(Args...) const, bool negate, Args... args){
  return std::bind(&condition,std::placeholders::_1,args...);
}

class MTRShuttle
{
  public:
    void parseRunList(std::string path="");
    void parseOCDB(std::string path="");
    void parseAMANDAiMon(std::string path = "");
//TODO:    void parseAMANDAVeff(std::string path = "");
    void propagateAMANDA();
    void saveData(std::string path = "MTRShuttle.csv");
    void loadData(std::string path = "MTRShuttle.csv");
    void computeAverage();

    template<typename XType, typename YType, typename CondType>
    TGraph *drawCorrelation(XType (RunObject::*getX)() const,
                            YType (RunObject::*getY)() const,
                            bool normalizeToAreaX,
                            bool normalizeToAreaY,
                            bool accumulate,
                            bool plotAverage,
                            int plane,
                            int side,
                            int RPC,
                            cond_vector conditions);

    template<typename XType, typename YType>
    TGraph *drawCorrelation(XType (RunObject::*getX)() const,
                            YType (RunObject::*getY)() const,
                            bool normalizeToAreaX,
                            bool normalizeToAreaY,
                            bool accumulate,
                            bool plotAverage,
                            int plane,
                            int side,
                            int RPC,
                            cond_type condition);

    template<typename XType, typename YType, typename CondType>
    TMultiGraph* drawCorrelations(XType(RunObject::*getX)() const,
                                  YType(RunObject::*getY)() const,
                                  bool normalizeToAreaX,
                                  bool normalizeToAreaY,
                                  bool accumulate,
                                  bool plotAverage,
                                  int MT,
                                  int side,
                                  CondType conditions);

    template<typename YType, typename CondType>
    TGraph *drawTrend(YType (RunObject::*getY)() const,
                      bool normalizeToArea,
                      bool accumulate,
                      bool plotAverage,
                      int plane,
                      int side,
                      int RPC,
                      CondType conditions);

    template<typename YType, typename CondType>
    TMultiGraph *drawTrends(YType (RunObject::*getY)() const,
                            bool normalizeToArea,
                            bool accumulate,
                            bool plotAverage,
                            int plane,
                            int side,
                            CondType conditions);

    template<typename YType, typename CondType>
    TMultiGraph* drawMaxMin(YType(RunObject::*getY)() const,
                            bool normalizeToArea,
                            bool accumulate,
                            bool plotAverage,
                            int plane,
                            int side,
                            CondType conditions);



  public:
    std::vector<std::pair<int,int>> fRunList;
    std::vector<RunObject> fRunDataVect[kNPlanes][kNSides][kNRPC];
    std::vector<RunObject> fRunDataVectAvg[kNPlanes+1];
    std::vector<AMANDACurrent> fAMANDACurrentsVect[kNPlanes][kNSides][kNRPC];

    inline double getM(const AMANDACurrent iStart, const AMANDACurrent iStop) {
      auto deltaI = iStop.getIDark()-iStart.getIDark();
      auto deltaTS = iStop.getTimeStamp()-iStart.getTimeStamp();
      return (deltaTS>0.)?deltaI/deltaTS:0.;
    };

    inline double getQ(const AMANDACurrent iStart, const AMANDACurrent /*iStop*/) {
      return iStart.getIDark();
    };

    void graphMaquillage(int plane, int RPC, TGraph *graph, bool isAvgGraph);

    TMultiGraph *interpreter(TString inputStr);
};

#endif //MTR_SHUTTLE_MTRSHUTTLE_H
