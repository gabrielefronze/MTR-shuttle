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
#include "RunObject.h"
#include "AMANDACurrent.h"
#include "Parameters.h"
#include "MTRConditions.h"

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

    #include "MTRShuttleTemplates.tcc"

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

//    TMultiGraph *interpreter(TString inputStr);
  ClassDef(MTRShuttle,1);
};

#endif //MTR_SHUTTLE_MTRSHUTTLE_H
