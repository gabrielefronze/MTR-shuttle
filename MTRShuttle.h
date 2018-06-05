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
#include "AMANDAVoltage.h"
#include "ReplacedRPC.h"
#include "Parameters.h"
#include "MTRConditions.h"
#include "Enumerators.h"


struct validityInterval {
  uint64_t start;
  uint64_t stop;
  validityInterval(uint64_t sta, uint64_t sto)
  {
    start = sta;
    stop = sto;
  }
};

class MTRShuttle
{
  friend class MTRBooster;

  private:
  template<class ContainerT>
  typename std::enable_if<std::is_same<std::string,typename ContainerT::value_type>::value>::type
  parseList(ContainerT paths, void (MTRShuttle::*parser)(std::string)){
    for(auto &itPaths : paths){
      (this->*parser)(itPaths);
    }
  }

  public:
  void instance(std::string runListPath, std::string AMANDAiMonPath, std::string AMANDAvMonPath, std::string OCDBPath, bool weighted=true, bool createFirstLast=true){
    this->parseRunList(runListPath);
    this->parseAMANDAvMon(AMANDAvMonPath);
    this->parseAMANDAiMon(AMANDAiMonPath);
    this->parseOCDB(OCDBPath);
    this->createDummyRuns(createFirstLast);
    this->propagateAMANDA(weighted);
  }

  void parseRunList(std::string path="");
  template<class ContainerT> void parseRunList(ContainerT paths){
    parseList(paths,&MTRShuttle::parseRunList);
  }

  void parseOCDB(std::string path="");
  void parseOCDBiMon(std::string path = "");

  void parseAMANDAiMon(std::string path = "");
  template<class ContainerT> void parseAMANDAvMon(ContainerT paths){
    parseList(paths,&MTRShuttle::parseAMANDAvMon);
  }

  void parseAMANDAvMon(std::string path = "");
  template<class ContainerT> void parseAMANDAiMon(ContainerT paths){
    parseList(paths,&MTRShuttle::parseAMANDAiMon);
  }

  void setAMANDAIsHVOk(int plane, int side, int RPC);
  void setAMANDAIsDark(int plane, int side, int RPC);
  void setAMANDAiDark(int plane, int side, int RPC);
  void propagateAMANDAVoltage(int plane, int side, int RPC, bool weightedAverage);
  void propagateAMANDACurrent(int plane, int side, int RPC, bool weightedAverage);

  void propagateAMANDA(bool weightedAverage = true);
  void saveData(std::string path = "MTRShuttle.csv");
  void loadData(std::string path = "MTRShuttle.csv");
  void computeAverage();

  private:
  std::vector<std::pair<int,int>> fRunList;
  std::vector<RunObject> fRunDataVect[MTRPlanes::kNPlanes][MTRSides::kNSides][MTRRPCs::kNRPCs];
  std::vector<RunObject> fRunDataVectAvg[MTRPlanes::kNPlanes+1];
  std::vector<AMANDACurrent> fAMANDACurrentsVect[MTRPlanes::kNPlanes][MTRSides::kNSides][MTRRPCs::kNRPCs];
  std::vector<AMANDAVoltage> fAMANDAVoltagesVect[MTRPlanes::kNPlanes][MTRSides::kNSides][MTRRPCs::kNRPCs];

  void createDummyRuns(bool createFirstLast);

  inline double getM(const AMANDACurrent iStart, const AMANDACurrent iStop) {
    auto deltaI = iStop.getIDark()-iStart.getIDark();
    auto deltaTS = iStop.getTimeStamp()-iStart.getTimeStamp();
    return (deltaTS>0.)?deltaI/deltaTS:0.;
  };

  inline double getQ(const AMANDACurrent iStart, const AMANDACurrent /*iStop*/) {
    return iStart.getIDark();
  };

  ClassDef(MTRShuttle,1);

  void setAMANDAIsHVOk();
};

#endif //MTR_SHUTTLE_MTRSHUTTLE_H
