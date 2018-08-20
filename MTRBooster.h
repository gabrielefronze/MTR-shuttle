//
// Created by Gabriele Gaetano Fronzé on 19/03/2018.
//

#ifndef MTR_SHUTTLE_MTRBOOSTER_H
#define MTR_SHUTTLE_MTRBOOSTER_H

#include <set>
#include <TVirtualPad.h>
#include "MTRShuttle.h"
#include "MTRConditions.h"
#include "Enumerators.h"

struct MTRPlotSettings{
  double (RunObject::*funcX)(MTRPlanes p, MTRSides s, MTRRPCs r) const = nullptr;
  double (RunObject::*funcY)(MTRPlanes p, MTRSides s, MTRRPCs r) const = nullptr;

  MTRPlanes fPlane=kNPlanes;
  MTRSides fSide=kNSides;
  MTRRPCs fRPC=kNRPCs;

  bool fNormalize=false;
  bool fAccumulate=false;
  bool fPlotaverage=false;

  bool isTrend=false;
  bool isMinMax=false;

  MTRConditions fConditions;

  bool fValidSettings=true;
};

class MTRBooster
{
  public:

//  MTRBooster();
  MTRBooster(std::string runListPath, std::string AMANDAiMonPath, std::string AMANDAvMonPath, std::string OCDBPath, bool weighted=true);
//  explicit MTRBooster(std::string inputData);
  explicit MTRBooster(MTRShuttle shuttle);
  inline MTRShuttle* GetShuttle(){ return &fShuttle; };
  void PrintConfig();
  void LoadReplacedRPCs(std::string path = "MTRRep.dat");

  void Launch();
  void Launch(size_t iSetting, TMultiGraph* buffer);
  inline TMultiGraph* GetPlot(size_t iPlot){ return (iPlot<fPlots.size())?fPlots[iPlot]:nullptr; };
  void AutoDraw(size_t iPlot, TVirtualPad* pad, bool drawLegend=false, Option_t* opt="alp", Option_t* legOpt="LP");

  MTRBooster& SetPlane(int HR_plane);
  MTRBooster& SetSide(std::string HR_side);
  MTRBooster& SetRPC(int HR_RPC);
  MTRBooster& SetX(std::string xValues);
  MTRBooster& SetY(std::string xValues);

  MTRBooster& SetTSRange(uint64_t minTS, uint64_t maxTS);
  MTRBooster& SetMinTS(uint64_t minTS);
  MTRBooster& SetMaxTS(uint64_t maxTS);

  MTRBooster& SetDateRange(std::string minDay, std::string maxDay);
  MTRBooster& SetMinDate(std::string minDay);
  MTRBooster& SetMaxDate(std::string maxDay);

  MTRBooster& SetRunRange(uint64_t minRun, uint64_t maxRun);
  MTRBooster& SetMinRun(uint64_t minRun);
  MTRBooster& SetMaxRun(uint64_t maxRun);

  MTRBooster& OnlyHVOkRuns();
  MTRBooster& OnlyWithBeamRuns();
  MTRBooster& OnlyNoBeamRuns();
  MTRBooster& OnlyDarkCurrentRuns();
  MTRBooster& OnlyIntegratedChargeRuns();

  inline MTRBooster& AccumulateY(){ fCurrentPlotSetting.fAccumulate=true; return *this; }
  inline MTRBooster& NormalizeToArea(){ fCurrentPlotSetting.fNormalize=true; return *this; }
  inline MTRBooster& PlotAverage(){ fCurrentPlotSetting.fPlotaverage=true; return *this; }
  inline MTRBooster& PlotMinMax(){ fCurrentPlotSetting.isMinMax=true; return *this; }

  inline void StackStage(){
    if( fCurrentPlotSetting.fValidSettings ){
      fPlotSettings.emplace_back(fCurrentPlotSetting);
      fCurrentPlotSetting=MTRPlotSettings();
    } else std::cerr << "Cannot stack invalid plot configuration.\n";
  }

#include "MTRBoosterTemplates.tcc"

  private:
  MTRShuttle fShuttle;
  bool fLoadedData;
  bool fAverageComputed;
  std::vector<MTRPlotSettings> fPlotSettings;
  MTRPlotSettings fCurrentPlotSetting;
  std::vector<TMultiGraph*> fPlots;
  std::vector<ReplacedRPC> fReplacedRPCs;

  inline void resetReplacedRPCs(){ for(auto &itRep : fReplacedRPCs){ itRep.fAlreadyReset=false; }};

  uint64_t getTSFromString(std::string date);
  void setXGetterFromString(std::string func);
  void setYGetterFromString(std::string func);

  void correlationWrapper(MTRPlotSettings &setting, TMultiGraph* buffer);
  void correlationsWrapper(MTRPlotSettings &setting, TMultiGraph* buffer);
  void trendWrapper(MTRPlotSettings &setting, TMultiGraph* buffer);
  void trendsWrapper(MTRPlotSettings &setting, TMultiGraph* buffer);
  void minmaxWrapper(MTRPlotSettings &setting, TMultiGraph* buffer);
};

#endif //MTR_SHUTTLE_MTRBOOSTER_H
