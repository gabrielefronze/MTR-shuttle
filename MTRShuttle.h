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


class PlotCondition{
  public:
    PlotCondition(bool (RunObject::*condition)(uint64_t,uint64_t) const, bool negate, uint64_t arg0=0, uint64_t arg1=0) : fNegate(negate){
      fCondition=condition;
      fArgs[0]=arg0;
      fArgs[1]=arg1;
    }
    bool operator()(RunObject data) const{
      return ((data.*(fCondition))(fArgs[0],fArgs[1]) == !fNegate);
    }
    bool operator()(RunObject *data) const{
      return operator()(*data);
    }

    bool (RunObject::*fCondition)(uint64_t,uint64_t) const;
    uint64_t fArgs[2];
    bool fNegate;
};
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
                            std::vector<PlotCondition> conditions);

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
                            PlotCondition condition);

    template<typename XType, typename YType, typename CondType>
    TMultiGraph* drawCorrelations(XType(RunObject::*getX)() const,
                                  YType(RunObject::*getY)() const,
                                  bool normalizeToAreaX,
                                  bool normalizeToAreaY,
                                  bool accumulate,
                                  bool plotAverage,
                                  int MT,
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
                            CondType conditions);

    template<typename YType, typename CondType>
    TMultiGraph* drawMaxMin(YType(RunObject::*getY)() const,
                            bool normalizeToArea,
                            bool accumulate,
                            bool plotAverage,
                            int M,
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
