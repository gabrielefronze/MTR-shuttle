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

    template<typename XType, typename YType> TGraph* drawCorrelation(int plane, int side, int RPC,
                                                               XType(RunObject::*getX)() const,
                                                               YType(RunObject::*getY)() const,
                                                               bool normalizeToArea);
    template<typename XType, typename YType> TMultiGraph* drawCorrelation(XType(RunObject::*getX)() const,
                                                                          YType(RunObject::*getY)() const,
                                                                    bool normalizeToArea);
    template<typename YType>  TGraph* drawTrend(int plane, int side, int RPC,
                                                YType(RunObject::*getY)() const,
                                                bool normalizeToArea);
    template<typename YType> TMultiGraph* drawTrend(YType(RunObject::*getY)() const,
                                                    bool normalizeToArea);

  public:
    std::vector<std::pair<int,int>> fRunList;
    std::vector<RunObject> fRunDataVect[kNPlanes][kNSides][kNRPC];
    std::vector<AMANDACurrent> fAMANDACurrentsVect[kNPlanes][kNSides][kNRPC];

    inline double getM(const AMANDACurrent iStart, const AMANDACurrent iStop) {
      auto deltaI = iStop.getIDark()-iStart.getIDark();
      auto deltaTS = iStop.getTimeStamp()-iStart.getTimeStamp();
      return (deltaTS>0.)?deltaI/deltaTS:0.;
    };

    inline double getQ(const AMANDACurrent iStart, const AMANDACurrent iStop) {
      return iStart.getIDark();
    };

    void graphMaquillage(int plane, int side, int RPC, TGraph* graph);
};

#endif //MTR_SHUTTLE_MTRSHUTTLE_H
