//
// Created by Gabriele Gaetano Fronzé on 19/03/2018.
//

#ifndef MTR_SHUTTLE_MTRBOOSTER_H
#define MTR_SHUTTLE_MTRBOOSTER_H

#include <set>
#include "MTRShuttle.h"

struct MTRPlotSettings{
    double (RunObject::*funcX)() const = nullptr;
    double (RunObject::*funcY)() const = nullptr;
    int fPlane=-1;
    int fSide=-1;
    int fRPC=-1;
    bool fNormalize=false;
    bool fAccumulate=false;
    bool fPlotaverage=false;
    MTRConditions fConditions;
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

    inline MTRBooster* AccumulateY(){ fCurrentPlotSetting.fAccumulate=true; return this; };
    inline MTRBooster* NormalizeToArea(){ fCurrentPlotSetting.fNormalize=true; return this; };
    inline MTRBooster* PlotAverage();

    inline void StackStage(){ fPlotSettings.emplace_back(fCurrentPlotSetting); fCurrentPlotSetting=MTRPlotSettings(); };


  private:
    MTRShuttle fShuttle;
    bool fLoadedData;
    bool fAverageComputed;
    std::vector<MTRPlotSettings> fPlotSettings;
    MTRPlotSettings fCurrentPlotSetting;

    uint64_t getTSFromString(std::string date);
    void setGetterFromString(std::string func, double (RunObject::*funcPtr)() const);
};

#endif //MTR_SHUTTLE_MTRBOOSTER_H
