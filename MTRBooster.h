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
    double (RunObject::*funcX)() const = nullptr;
    double (RunObject::*funcY)() const = nullptr;
    MTRPlanes fPlane=kAll;
    MTRSides fSide=kBoth;
    MTRRPCs fRPC=kAllRPCs;
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

    MTRBooster();
    explicit MTRBooster(std::string inputData);
    inline MTRShuttle* getShuttle(){ return &fShuttle; };
    void PrintConfig();

    void Launch();
    void Launch(size_t iLaunch, TMultiGraph* buffer);
    void AutoDraw(size_t iLaunch, TVirtualPad* pad, Option_t* opt, bool drawLegend=false, Option_t* legOpt="P");

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
    inline MTRBooster& PlotAverage();
    inline void StackStage(){
      if( fCurrentPlotSetting.fValidSettings ){
        fPlotSettings.emplace_back(fCurrentPlotSetting);
        fCurrentPlotSetting=MTRPlotSettings();
      } else std::cerr << "Cannot stack invalid plot configuration.\n";
    }

  private:
    MTRShuttle fShuttle;
    bool fLoadedData;
    bool fAverageComputed;
    std::vector<MTRPlotSettings> fPlotSettings;
    MTRPlotSettings fCurrentPlotSetting;
    std::vector<TMultiGraph*> fPlots;
    inline void loadAverage(){ fShuttle.computeAverage(); fAverageComputed=true; }

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
