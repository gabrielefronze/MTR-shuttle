//
// Created by Gabriele Gaetano Fronzé on 19/03/2018.
//

#include <utility>

#include "MTRBooster.h"

MTRBooster::MTRBooster() : fLoadedData(false), fAverageComputed(false)
{
  std::string inputData;
  std::cout << "No input file path provided. Please provide a valid .csv file path below:\n";
  std::cin >> inputData;
  fShuttle.loadData(std::move(inputData));
  fLoadedData=true;
}

MTRBooster::MTRBooster(std::string inputData) : fLoadedData(false), fAverageComputed(false)
{
  fShuttle.loadData(std::move(inputData));
  fLoadedData=true;
}

std::vector<TMultiGraph *> MTRBooster::Launch()
{
  std::vector<TMultiGraph *> mgVector;

  mgVector.reserve(fPlotSettings.size());

  for (size_t iSetting = 0; iSetting < fPlotSettings.size(); ++iSetting) {
    mgVector.emplace_back(MTRBooster::Launch(iSetting));
  }

  return mgVector;
}

TMultiGraph *MTRBooster::Launch(size_t iLaunch)
{
  if( !fLoadedData ){
    std::cerr << "Input data not defined. Aborting.\n";
    return nullptr;
  }

  if( iLaunch >= fPlotSettings.size() ) {
    std::cerr << "Plot settings index out of range. Aborting.\n";
    return nullptr;
  }
  else {
    auto iSetting = &(fPlotSettings[iLaunch]);

    if( !iSetting->fValidSettings ){
      std::cerr << "Configuration " << iLaunch << " is not valid! Aborting.\n";
      return nullptr;
    }

    if( !fAverageComputed && iSetting->fPlotaverage ) MTRBooster::loadAverage();

    if( fCurrentPlotSetting.fRPC>=0 && fCurrentPlotSetting.fSide>=0 && fCurrentPlotSetting.fPlane>=0  ){
      if( iSetting->isTrend ){
        std::cout << "It's a single trend.\n";
        return trendWrapper(iSetting);
      } else {
        std::cout << "It's a single correlation.\n";
        return correlationWrapper(iSetting);
      }
    } else {
      if ( iSetting->isTrend ) {
        std::cout << "It's a multiple trend.\n";
        return trendsWrapper(iSetting);
      } else if ( iSetting->isMinMax ){
        std::cout << "It's a min-max trend.\n";
        return minmaxWrapper(iSetting);
      } else {
        std::cout << "It's a multiple correlation.\n";
        return correlationsWrapper(iSetting);
      }
    }
  }
}

MTRBooster *MTRBooster::SetPlane(int HR_plane)
{
  switch (HR_plane){
    case 1  : fCurrentPlotSetting.fPlane=MTRPlanes::kMT11; break;
    case 2  : fCurrentPlotSetting.fPlane=MTRPlanes::kMT12; break;
    case 3  : fCurrentPlotSetting.fPlane=MTRPlanes::kMT21; break;
    case 4  : fCurrentPlotSetting.fPlane=MTRPlanes::kMT22; break;
    case 11 : fCurrentPlotSetting.fPlane=MTRPlanes::kMT11; break;
    case 12 : fCurrentPlotSetting.fPlane=MTRPlanes::kMT12; break;
    case 21 : fCurrentPlotSetting.fPlane=MTRPlanes::kMT21; break;
    case 22 : fCurrentPlotSetting.fPlane=MTRPlanes::kMT22; break;
    case 13 : fCurrentPlotSetting.fPlane=MTRPlanes::kMT21; break;
    case 14 : fCurrentPlotSetting.fPlane=MTRPlanes::kMT22; break;
    default : fCurrentPlotSetting.fPlane=MTRPlanes::kAll;  break;
  }

  if( fCurrentPlotSetting.fPlane==kAll ) std::cerr << "Plane ID not recognised. Plotting all planes.\n";

  return this;
}

MTRBooster *MTRBooster::SetSide(std::string HR_side)
{
  if( HR_side.find("out")!=std::string::npos ) fCurrentPlotSetting.fSide=MTRSides::kOUTSIDE;
  else if( HR_side.find("in")!=std::string::npos ) fCurrentPlotSetting.fSide=MTRSides::kINSIDE;
  else if( HR_side.find("OUT")!=std::string::npos ) fCurrentPlotSetting.fSide=MTRSides::kOUTSIDE;
  else if( HR_side.find("IN")!=std::string::npos ) fCurrentPlotSetting.fSide=MTRSides::kINSIDE;
  else fCurrentPlotSetting.fSide=MTRSides::kBoth;

  if( fCurrentPlotSetting.fSide==kBoth ) std::cerr << "Side not recognised. Plotting all sides.\n";

  return this;
}

MTRBooster *MTRBooster::SetRPC(int HR_RPC)
{
  if( HR_RPC<=MTRRPCs::kNRPCs && HR_RPC>=1 ) fCurrentPlotSetting.fRPC=(MTRRPCs)(HR_RPC-1);
  else fCurrentPlotSetting.fRPC=kAllRPCs;

  if( fCurrentPlotSetting.fRPC==kAllRPCs ) std::cerr << "Wrong RPC ID. Plotting all RPCs.\n";

  return this;
}

MTRBooster *MTRBooster::SetX(std::string xValues)
{
  setGetterFromString(xValues, funcOpt::kX);
  return this;
}

MTRBooster *MTRBooster::SetY(std::string yValues)
{
  setGetterFromString(yValues, funcOpt::kY);
  return this;
}

MTRBooster *MTRBooster::SetTSRange(uint64_t minTS, uint64_t maxTS)
{
  fCurrentPlotSetting.fConditions.addCondition(&RunObject::isBetweenTimestamps,false,minTS,maxTS);
  return this;
}

MTRBooster *MTRBooster::SetMinTS(uint64_t minTS)
{
  fCurrentPlotSetting.fConditions.addCondition(&RunObject::isAfter,false,minTS);
  return this;
}

MTRBooster *MTRBooster::SetMaxTS(uint64_t maxTS)
{
  fCurrentPlotSetting.fConditions.addCondition(&RunObject::isBefore,false,maxTS);
  return this;
}

MTRBooster *MTRBooster::SetDateRange(std::string minDay, std::string maxDay)
{
  return SetTSRange(getTSFromString(std::move(minDay)),getTSFromString(std::move(maxDay)));
}

MTRBooster *MTRBooster::SetMinDate(std::string minDay)
{
  return SetMinTS(getTSFromString(std::move(minDay)));
}

MTRBooster *MTRBooster::SetMaxDate(std::string maxDay)
{
  return SetMaxTS(getTSFromString(std::move(maxDay)));
}

MTRBooster *MTRBooster::SetRunRange(uint64_t minRun, uint64_t maxRun)
{
  fCurrentPlotSetting.fConditions.addCondition(&RunObject::isBetweenRuns,false,minRun,maxRun);
  return this;
}

MTRBooster *MTRBooster::SetMinRun(uint64_t minRun)
{
  fCurrentPlotSetting.fConditions.addCondition(&RunObject::isAfterRun,false,minRun);
  return this;
}

MTRBooster *MTRBooster::SetMaxRun(uint64_t maxRun)
{
  fCurrentPlotSetting.fConditions.addCondition(&RunObject::isBeforeRun,false,maxRun);
  return this;
}

MTRBooster *MTRBooster::OnlyHVOkRuns()
{
  fCurrentPlotSetting.fConditions.addCondition(&RunObject::isHVOk,false);
  return this;
}

MTRBooster *MTRBooster::OnlyWithBeamRuns()
{
  fCurrentPlotSetting.fConditions.addCondition(&RunObject::isDark,true);
  return this;
}

MTRBooster *MTRBooster::OnlyNoBeamRuns()
{
  fCurrentPlotSetting.fConditions.addCondition(&RunObject::isDark,false);
  return this;
}

MTRBooster *MTRBooster::OnlyDarkCurrentRuns()
{
  fCurrentPlotSetting.fConditions.addCondition(&RunObject::isValidForIDark,false);
  return this;
}

MTRBooster *MTRBooster::OnlyIntegratedChargeRuns()
{
  fCurrentPlotSetting.fConditions.addCondition(&RunObject::isValidForIntCharge,false);
  return this;
}

MTRBooster *MTRBooster::PlotAverage()
{
  fCurrentPlotSetting.fPlotaverage=true;
  return this;
}

uint64_t MTRBooster::getTSFromString(std::string date)
{
  std::tm timeStruct{};
  std::tm timeStructGMT{};
  int dummyYear;

  sscanf(date.c_str(), "%d/%d/%d",&(timeStruct.tm_mday),&(timeStruct.tm_mon),&dummyYear);

  timeStruct.tm_year=(dummyYear>2000)?dummyYear:dummyYear+2000;
  timeStruct.tm_year-=1900; //First epoch year is 1900
  timeStruct.tm_hour++; //CERN is at GMT+1
  timeStruct.tm_mon--; //Epoch starts from month=0 which corresponds to January

  time_t timeGMT=timegm(&timeStruct);
  timeStructGMT=*(gmtime(&timeGMT));

  return (uint64_t)std::mktime(&timeStructGMT);
}

void MTRBooster::setGetterFromString(std::string func, funcOpt opt)
{

  auto funcPtr = (opt==funcOpt::kX)?fCurrentPlotSetting.funcX:fCurrentPlotSetting.funcY;

  if ( func == "IDark" ) funcPtr = &RunObject::getAvgIDark;
  else if ( func == "ITot" ) funcPtr = &RunObject::getAvgITot;
  else if ( func == "INet" ) funcPtr = &RunObject::getAvgINet;
  else if ( func == "HV" ) funcPtr = &RunObject::getAvgHV;
  else if ( func == "RateBend" ) funcPtr = &RunObject::getScalBending;
  else if ( func == "RateNotBend" ) funcPtr = &RunObject::getScalNotBending;
  else if ( func == "IntCharge" ) funcPtr = &RunObject::getIntCharge;
  else if ( func == "Time" ) {
    fCurrentPlotSetting.funcX = nullptr;
    if ( opt==funcOpt ::kX ) fCurrentPlotSetting.isTrend = true;
    else {
      std::cerr << "Time can only be used as X of a graph. The configuration is invalid and will be skipped.";
      fCurrentPlotSetting.fValidSettings = false;
    }
  }
  else {
    funcPtr = nullptr;
    fCurrentPlotSetting.fValidSettings = false;
    std::cerr << "Requested function specifier " << func << " has not been recognised as valid. Configuration will be skipped.\n";
    return;
  }

  fCurrentPlotSetting.fValidSettings = true;
}

TMultiGraph *MTRBooster::correlationWrapper(MTRPlotSettings *setting)
{
  auto returnedMG = new TMultiGraph();
  returnedMG->Add(fShuttle.drawCorrelation(setting->funcX,
                                           setting->funcY,
                                           setting->fNormalize,
                                           setting->fNormalize,
                                           setting->fAccumulate,
                                           setting->fPlotaverage,
                                           setting->fPlane,
                                           setting->fSide,
                                           setting->fRPC,
                                           setting->fConditions));
  return returnedMG;
}

TMultiGraph *MTRBooster::correlationsWrapper(MTRPlotSettings *setting)
{
  return fShuttle.drawCorrelations(setting->funcX,
                                   setting->funcY,
                                   setting->fNormalize,
                                   setting->fNormalize,
                                   setting->fAccumulate,
                                   setting->fPlotaverage,
                                   setting->fPlane,
                                   setting->fSide,
                                   setting->fConditions);
}

TMultiGraph *MTRBooster::trendWrapper(MTRPlotSettings *setting)
{
  auto returnedMG = new TMultiGraph();
  returnedMG->Add(fShuttle.drawTrend(setting->funcY,
                                     setting->fNormalize,
                                     setting->fAccumulate,
                                     setting->fPlotaverage,
                                     setting->fPlane,
                                     setting->fSide,
                                     setting->fRPC,
                                     setting->fConditions));
  return returnedMG;
}

TMultiGraph *MTRBooster::trendsWrapper(MTRPlotSettings *setting)
{
  return fShuttle.drawTrends(setting->funcY,
                             setting->fNormalize,
                             setting->fAccumulate,
                             setting->fPlotaverage,
                             setting->fPlane,
                             setting->fSide,
                             setting->fConditions);
}

TMultiGraph *MTRBooster::minmaxWrapper(MTRPlotSettings *setting)
{
  return fShuttle.drawMaxMin(setting->funcY,
                             setting->fNormalize,
                             setting->fAccumulate,
                             setting->fPlotaverage,
                             setting->fPlane,
                             setting->fSide,
                             setting->fConditions);
}
