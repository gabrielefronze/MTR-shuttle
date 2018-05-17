//
// Created by Gabriele Gaetano Fronzé on 01/02/2018.
//

#ifndef MTR_SHUTTLE_MTRSHUTTLE_H
#define MTR_SHUTTLE_MTRSHUTTLE_H

#include <vector>
#include <string>
#include <TGraph.h>
#include <TMultiGraph.h>
#include <TClonesArray.h>
#include <TAxis.h>
#include <TStyle.h>
#include <TH1F.h>
#include <TList.h>
#include "RunObject.h"
#include "AMANDACurrent.h"
#include "Parameters.h"
#include "MTRConditions.h"
#include "Enumerators.h"

class MTRShuttle
{
  public:
    void parseRunList(std::string path="");
    void parseOCDB(std::string path="");
    void parseAMANDAiMon(std::string path = "");
    void parseOCDBiMon(std::string path = "");
//TODO:    void parseAMANDAVeff(std::string path = "");
  void propagateAMANDA(bool weightedAverage = true);
    void saveData(std::string path = "MTRShuttle.csv");
    void loadData(std::string path = "MTRShuttle.csv");
    void computeAverage();

    #include "MTRShuttleTemplates.tcc"

  public:
    std::vector<std::pair<int,int>> fRunList;
    std::vector<RunObject> fRunDataVect[MTRPlanes::kNPlanes][MTRSides::kNSides][MTRRPCs::kNRPCs];
    std::vector<RunObject> fRunDataVectAvg[MTRPlanes::kNPlanes+1];
    std::vector<AMANDACurrent> fAMANDACurrentsVect[MTRPlanes::kNPlanes][MTRSides::kNSides][MTRRPCs::kNRPCs];

    inline double getM(const AMANDACurrent iStart, const AMANDACurrent iStop) {
      auto deltaI = iStop.getIDark()-iStart.getIDark();
      auto deltaTS = iStop.getTimeStamp()-iStart.getTimeStamp();
      return (deltaTS>0.)?deltaI/deltaTS:0.;
    };

    inline double getQ(const AMANDACurrent iStart, const AMANDACurrent /*iStop*/) {
      return iStart.getIDark();
    };

    void graphMaquillage(MTRPlanes plane, MTRRPCs RPC, TGraph *graph, bool isAvgGraph);

  ClassDef(MTRShuttle,1);
};

#endif //MTR_SHUTTLE_MTRSHUTTLE_H
