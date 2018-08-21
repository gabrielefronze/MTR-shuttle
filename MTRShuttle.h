//
// Created by Gabriele Gaetano Fronzé on 01/02/2018.
//

#ifndef MTR_SHUTTLE_MTRSHUTTLE_H
#define MTR_SHUTTLE_MTRSHUTTLE_H

#include <utility>
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
  template<typename RLT, typename iMonT, typename vMonT>
  void instance(RLT runListPath, iMonT AMANDAiMonPath, vMonT AMANDAvMonPath, std::string OCDBPath, bool weighted=true, bool createFirstLast=true){
    this->parseRunList(std::move(runListPath));
    this->parseAMANDAvMon(std::move(AMANDAvMonPath));
    this->parseAMANDAiMon(std::move(AMANDAiMonPath));
    this->parseOCDB(std::move(OCDBPath));
    this->createDummyRuns(createFirstLast);
    this->propagateAMANDA(weighted);
  }

  void parseRunList(std::string path="");
  template<class ContainerT> 
  typename std::enable_if<!(std::is_same<ContainerT,std::string>::value && std::is_same<typename ContainerT::value_type,std::string>::value),void>::type parseRunList(ContainerT paths){
    parseList(paths,&MTRShuttle::parseRunList);
  }

  void parseOCDB(std::string path="");
  void parseOCDBiMon(std::string path = "");

  void parseAMANDAiMon(std::string path = "");
  template<class ContainerT> 
  typename std::enable_if<!(std::is_same<ContainerT,std::string>::value && std::is_same<typename ContainerT::value_type,std::string>::value),void>::type parseAMANDAvMon(ContainerT paths){
    parseList(paths,&MTRShuttle::parseAMANDAvMon);
  }

  void parseAMANDAvMon(std::string path = "");
  template<class ContainerT> 
  typename std::enable_if<!(std::is_same<ContainerT,std::string>::value && std::is_same<typename ContainerT::value_type,std::string>::value),void>::type parseAMANDAiMon(ContainerT paths){
    parseList(paths,&MTRShuttle::parseAMANDAiMon);
  }

  void setAMANDAIsHVOk(int plane, int side, int RPC);
  void setAMANDAIsDark(int plane, int side, int RPC);
  void setAMANDAiDark(int plane, int side, int RPC);
  void propagateAMANDAVoltage(int plane, int side, int RPC, bool weightedAverage);
  void propagateAMANDACurrent(int plane, int side, int RPC, bool weightedAverage);

  void propagateAMANDA(bool weightedAverage = true);
//  void saveData(std::string path = "MTRShuttle.csv");
//  void loadData(std::string path = "MTRShuttle.csv");
//  void computeAverage();

  private:
  std::vector<std::pair<int,int>> fRunList;
  public:
  std::vector<RunObject> fRunDataVect;
  std::vector<AMANDACurrent> fAMANDACurrentsVect[MTRPlanes::kNPlanes][MTRSides::kNSides][MTRRPCs::kNRPCs];
  std::vector<AMANDAVoltage> fAMANDAVoltagesVect[MTRPlanes::kNPlanes][MTRSides::kNSides][MTRRPCs::kNRPCs];

  private:
  void createDummyRunsInRange(uint64_t &firstRunNumber, uint64_t SOR, uint64_t EOR);
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
};

#endif //MTR_SHUTTLE_MTRSHUTTLE_H
