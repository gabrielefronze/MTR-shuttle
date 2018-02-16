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

    template<typename XType, typename YType, class ...Args>
    TGraph *drawCorrelation(XType (RunObject::*getX)() const,
                            YType (RunObject::*getY)() const,
                            bool normalizeToAreaX=false,
                            bool normalizeToAreaY=false,
                            bool accumulate=false,
                            bool plotAverage=true,
                            int plane=-1,
                            int side=-1,
                            int RPC=-1,
                            bool negateCondition=false,
                            bool (RunObject::*condition)(Args... args) const = &RunObject::getTrue,
                            Args... args);

    template<typename XType, typename YType, class ...Args>
    TMultiGraph* drawCorrelations(XType(RunObject::*getX)() const,
                                  YType(RunObject::*getY)() const,
                                  bool normalizeToAreaX,
                                  bool normalizeToAreaY,
                                  bool accumulate,
                                  bool plotAverage,
                                  int MT,
                                  bool negateCondition,
                                  bool (RunObject::*condition)(Args... args) const,
                                  Args... args);

    template<typename YType, class ...Args>
    TGraph *drawTrend(YType (RunObject::*getY)() const,
                      bool normalizeToArea,
                      bool accumulate,
                      bool plotAverage,
                      int plane,
                      int side,
                      int RPC,
                      bool negateCondition,
                      bool (RunObject::*condition)(Args... args) const,
                      Args... args);

    template<typename YType, class ...Args>
    TMultiGraph *drawTrends(YType (RunObject::*getY)() const,
                            bool normalizeToArea,
                            bool accumulate,
                            bool plotAverage,
                            int plane,
                            bool negateCondition,
                            bool (RunObject::*condition)(Args... args) const,
                            Args... args);

    template<typename YType, class ...Args>
    TMultiGraph* drawMaxMin(YType(RunObject::*getY)() const,
                            bool normalizeToArea,
                            bool accumulate,
                            bool plotAverage,
                            int M,
                            bool negateCondition,
                            bool (RunObject::*condition)(Args... args) const,
                            Args... args);

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

    inline double getQ(const AMANDACurrent iStart, const AMANDACurrent iStop) {
      return iStart.getIDark();
    };

    void graphMaquillage(int plane, int RPC, TGraph *graph, bool isAvgGraph);
};

#endif //MTR_SHUTTLE_MTRSHUTTLE_H
