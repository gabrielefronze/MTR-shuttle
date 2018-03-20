//
// Created by Gabriele Gaetano Fronzé on 19/03/2018.
//

#ifndef MTR_SHUTTLE_MTRBOOSTER_H
#define MTR_SHUTTLE_MTRBOOSTER_H

#include <set>
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

enum funcOpt{
    kX,
    kY
};

class MTRBooster
{
  public:

    MTRBooster();
    explicit MTRBooster(std::string inputData);
    inline MTRShuttle* getShuttle(){ return &fShuttle; };
    std::vector<TMultiGraph*> Launch();

    TMultiGraph* Launch(size_t iLaunch);
    MTRBooster* SetPlane(int HR_plane);

    MTRBooster* SetSide(std::string HR_side);
    MTRBooster* SetRPC(int HR_RPC);
    MTRBooster* SetX(std::string xValues);

    MTRBooster* SetY(std::string xValues);
    MTRBooster* SetTSRange(uint64_t minTS, uint64_t maxTS);

    MTRBooster* SetMinTS(uint64_t minTS);
    MTRBooster* SetMaxTS(uint64_t maxTS);
    MTRBooster* SetDateRange(std::string minDay, std::string maxDay);

    MTRBooster* SetMinDate(std::string minDay);
    MTRBooster* SetMaxDate(std::string maxDay);
    MTRBooster* SetRunRange(uint64_t minRun, uint64_t maxRun);

    MTRBooster* SetMinRun(uint64_t minRun);
    MTRBooster* SetMaxRun(uint64_t maxRun);

    MTRBooster* OnlyHVOkRuns();
    MTRBooster* OnlyWithBeamRuns();
    MTRBooster* OnlyNoBeamRuns();
    MTRBooster* OnlyDarkCurrentRuns();
    MTRBooster* OnlyIntegratedChargeRuns();
    inline MTRBooster* AccumulateY(){ fCurrentPlotSetting.fAccumulate=true; return this; }

    inline MTRBooster* NormalizeToArea(){ fCurrentPlotSetting.fNormalize=true; return this; }
    inline MTRBooster* PlotAverage();
    inline void StackStage(){ fPlotSettings.emplace_back(fCurrentPlotSetting); fCurrentPlotSetting=MTRPlotSettings(); }

  private:

    MTRShuttle fShuttle;
    bool fLoadedData;
    bool fAverageComputed;
    std::vector<MTRPlotSettings> fPlotSettings;
    MTRPlotSettings fCurrentPlotSetting;
    inline void loadAverage(){ fShuttle.computeAverage(); fAverageComputed=true; }

    uint64_t getTSFromString(std::string date);
    void setGetterFromString(std::string func, funcOpt opt);

    TMultiGraph* correlationWrapper(MTRPlotSettings *setting);
    TMultiGraph* correlationsWrapper(MTRPlotSettings *setting);
    TMultiGraph* trendWrapper(MTRPlotSettings *setting);
    TMultiGraph* trendsWrapper(MTRPlotSettings *setting);
    TMultiGraph* minmaxWrapper(MTRPlotSettings *setting);
};

#endif //MTR_SHUTTLE_MTRBOOSTER_H
