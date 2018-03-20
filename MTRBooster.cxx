//
// Created by Gabriele Gaetano Fronzé on 19/03/2018.
//

#include "MTRBooster.h"
#include "TCanvas.h"
#include "RunObject.h"
#include <utility>

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
    mgVector.emplace_back(new TMultiGraph(Form("%zu",iSetting),Form("%zu",iSetting)));
    MTRBooster::Launch(iSetting,mgVector.back());
  }

  return mgVector;
}

void MTRBooster::Launch(size_t iLaunch, TMultiGraph* buffer)
{
  if( !fLoadedData ){
    std::cerr << "Input data not defined. Aborting.\n";
    return;
  }

  if( iLaunch >= fPlotSettings.size() ) {
    std::cerr << "Plot settings index out of range. Aborting.\n";
    return;
  }
  else {
    auto iSetting = (fPlotSettings[iLaunch]);

    std::cout<<"getX is "<< iSetting.funcX <<"\n";
    std::cout<<"getY is "<< iSetting.funcY <<"\n";

    if( !iSetting.fValidSettings ){
      std::cerr << "Configuration " << iLaunch << " is not valid! Aborting.\n";
      return;
    }

    if( !fAverageComputed && iSetting.fPlotaverage ) MTRBooster::loadAverage();

    if( fCurrentPlotSetting.fRPC>=0 && fCurrentPlotSetting.fSide>=0 && fCurrentPlotSetting.fPlane>=0  ){
      if( iSetting.isTrend ){
        std::cout << "It's a single trend.\n";
        trendWrapper(iSetting, buffer);
        std::cout<<"DONE\n";
      } else {
        std::cout << "It's a single correlation.\n";
        correlationWrapper(iSetting, buffer);
        std::cout<<"DONE\n";
      }
    } else {
      if ( iSetting.isTrend ) {
        std::cout << "It's a multiple trend.\n";
        trendsWrapper(iSetting, buffer);
        std::cout<<"DONE\n";
      } else if ( iSetting.isMinMax ){
        std::cout << "It's a min-max trend.\n";
        minmaxWrapper(iSetting, buffer);
        std::cout<<"DONE\n";
      } else {
        std::cout << "It's a multiple correlation.\n";
        correlationsWrapper(iSetting, buffer);
        std::cout<<"DONE\n";
      }
    }

    return;
  }
}

void MTRBooster::PrintConfig()
{
  printf("\n");
}

MTRBooster& MTRBooster::SetPlane(int HR_plane)
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

  return *this;
}

MTRBooster& MTRBooster::SetSide(std::string HR_side)
{
  if( HR_side.find("out")!=std::string::npos ) fCurrentPlotSetting.fSide=MTRSides::kOUTSIDE;
  else if( HR_side.find("in")!=std::string::npos ) fCurrentPlotSetting.fSide=MTRSides::kINSIDE;
  else if( HR_side.find("OUT")!=std::string::npos ) fCurrentPlotSetting.fSide=MTRSides::kOUTSIDE;
  else if( HR_side.find("IN")!=std::string::npos ) fCurrentPlotSetting.fSide=MTRSides::kINSIDE;
  else fCurrentPlotSetting.fSide=MTRSides::kBoth;

  if( fCurrentPlotSetting.fSide==kBoth ) std::cerr << "Side not recognised. Plotting all sides.\n";

  return *this;
}

MTRBooster& MTRBooster::SetRPC(int HR_RPC)
{
  if( HR_RPC<=MTRRPCs::kNRPCs && HR_RPC>=1 ) fCurrentPlotSetting.fRPC=(MTRRPCs)(HR_RPC-1);
  else fCurrentPlotSetting.fRPC=kAllRPCs;

  if( fCurrentPlotSetting.fRPC==kAllRPCs ) std::cerr << "Wrong RPC ID. Plotting all RPCs.\n";

  return *this;
}

MTRBooster& MTRBooster::SetX(std::string xValues)
{
  setXGetterFromString(xValues);
  return *this;
}

MTRBooster& MTRBooster::SetY(std::string yValues)
{
  setYGetterFromString(yValues);
  return *this;
}

MTRBooster& MTRBooster::SetTSRange(uint64_t minTS, uint64_t maxTS)
{
  fCurrentPlotSetting.fConditions.addCondition(&RunObject::isBetweenTimestamps,false,minTS,maxTS);
  return *this;
}

MTRBooster& MTRBooster::SetMinTS(uint64_t minTS)
{
  fCurrentPlotSetting.fConditions.addCondition(&RunObject::isAfter,false,minTS);
  return *this;
}

MTRBooster& MTRBooster::SetMaxTS(uint64_t maxTS)
{
  fCurrentPlotSetting.fConditions.addCondition(&RunObject::isBefore,false,maxTS);
  return *this;
}

MTRBooster& MTRBooster::SetDateRange(std::string minDay, std::string maxDay)
{
  return SetTSRange(getTSFromString(std::move(minDay)),getTSFromString(std::move(maxDay)));
}

MTRBooster& MTRBooster::SetMinDate(std::string minDay)
{
  return SetMinTS(getTSFromString(std::move(minDay)));
}

MTRBooster& MTRBooster::SetMaxDate(std::string maxDay)
{
  return SetMaxTS(getTSFromString(std::move(maxDay)));
}

MTRBooster& MTRBooster::SetRunRange(uint64_t minRun, uint64_t maxRun)
{
  fCurrentPlotSetting.fConditions.addCondition(&RunObject::isBetweenRuns,false,minRun,maxRun);
  return *this;
}

MTRBooster& MTRBooster::SetMinRun(uint64_t minRun)
{
  fCurrentPlotSetting.fConditions.addCondition(&RunObject::isAfterRun,false,minRun);
  return *this;
}

MTRBooster& MTRBooster::SetMaxRun(uint64_t maxRun)
{
  fCurrentPlotSetting.fConditions.addCondition(&RunObject::isBeforeRun,false,maxRun);
  return *this;
}

MTRBooster& MTRBooster::OnlyHVOkRuns()
{
  fCurrentPlotSetting.fConditions.addCondition(&RunObject::isHVOk,false);
  return *this;
}

MTRBooster& MTRBooster::OnlyWithBeamRuns()
{
  fCurrentPlotSetting.fConditions.addCondition(&RunObject::isDark,true);
  return *this;
}

MTRBooster& MTRBooster::OnlyNoBeamRuns()
{
  fCurrentPlotSetting.fConditions.addCondition(&RunObject::isDark,false);
  return *this;
}

MTRBooster& MTRBooster::OnlyDarkCurrentRuns()
{
  fCurrentPlotSetting.fConditions.addCondition(&RunObject::isValidForIDark,false);
  return *this;
}

MTRBooster& MTRBooster::OnlyIntegratedChargeRuns()
{
  fCurrentPlotSetting.fConditions.addCondition(&RunObject::isValidForIntCharge,false);
  return *this;
}

MTRBooster& MTRBooster::PlotAverage()
{
  fCurrentPlotSetting.fPlotaverage=true;
  return *this;
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

void MTRBooster::setXGetterFromString(std::string func)
{
  if ( func == "IDark" ) fCurrentPlotSetting.funcX = &RunObject::getAvgIDark;
  else if ( func == "ITot" ) fCurrentPlotSetting.funcX = &RunObject::getAvgITot;
  else if ( func == "INet" ) fCurrentPlotSetting.funcX = &RunObject::getAvgINet;
  else if ( func == "HV" ) fCurrentPlotSetting.funcX = &RunObject::getAvgHV;
  else if ( func == "RateBend" ) fCurrentPlotSetting.funcX = &RunObject::getScalBending;
  else if ( func == "RateNotBend" ) fCurrentPlotSetting.funcX = &RunObject::getScalNotBending;
  else if ( func == "IntCharge" ) fCurrentPlotSetting.funcX = &RunObject::getIntCharge;
  else if ( func == "Time" ) {
    fCurrentPlotSetting.funcX = nullptr;
    fCurrentPlotSetting.isTrend = true;
  }
  else {
    fCurrentPlotSetting.funcX = nullptr;
    fCurrentPlotSetting.fValidSettings = false;
    std::cerr << "Requested function specifier " << func << " has not been recognised as valid. Configuration will be skipped.\n";
    return;
  }

  fCurrentPlotSetting.fValidSettings = true;
}

void MTRBooster::setYGetterFromString(std::string func)
{
  if ( func == "IDark" ) fCurrentPlotSetting.funcY = &RunObject::getAvgIDark;
  else if ( func == "ITot" ) fCurrentPlotSetting.funcY = &RunObject::getAvgITot;
  else if ( func == "INet" ) fCurrentPlotSetting.funcY = &RunObject::getAvgINet;
  else if ( func == "HV" ) fCurrentPlotSetting.funcY = &RunObject::getAvgHV;
  else if ( func == "RateBend" ) fCurrentPlotSetting.funcY = &RunObject::getScalBending;
  else if ( func == "RateNotBend" ) fCurrentPlotSetting.funcY = &RunObject::getScalNotBending;
  else if ( func == "IntCharge" ) fCurrentPlotSetting.funcY = &RunObject::getIntCharge;
  else if ( func == "Time" ) {
    std::cerr << "Time can only be used as X of a graph. The configuration is invalid and will be skipped.";
    fCurrentPlotSetting.fValidSettings = false;
  }
  else {
    fCurrentPlotSetting.funcY = nullptr;
    fCurrentPlotSetting.fValidSettings = false;
    std::cerr << "Requested function specifier " << func << " has not been recognised as valid. Configuration will be skipped.\n";
    return;
  }

  fCurrentPlotSetting.fValidSettings = true;
}

void MTRBooster::correlationWrapper(MTRPlotSettings &setting, TMultiGraph* buffer)
{
  buffer->Add(fShuttle.drawCorrelation(setting.funcX,
                                       setting.funcY,
                                       setting.fNormalize,
                                       setting.fNormalize,
                                       setting.fAccumulate,
                                       setting.fPlotaverage,
                                       setting.fPlane,
                                       setting.fSide,
                                       setting.fRPC,
                                       setting.fConditions));

}

void MTRBooster::correlationsWrapper(MTRPlotSettings &setting, TMultiGraph* buffer)
{
//  std::cout<<"START\n";
  buffer = fShuttle.drawCorrelations(setting.funcX,
                                     setting.funcY,
                                     setting.fNormalize,
                                     setting.fNormalize,
                                     setting.fAccumulate,
                                     setting.fPlotaverage,
                                     setting.fPlane,
                                     setting.fSide,
                                     setting.fConditions);
  return;
}

void MTRBooster::trendWrapper(MTRPlotSettings &setting, TMultiGraph* buffer)
{
  buffer->Add(fShuttle.drawTrend(setting.funcY,
                                 setting.fNormalize,
                                 setting.fAccumulate,
                                 setting.fPlotaverage,
                                 setting.fPlane,
                                 setting.fSide,
                                 setting.fRPC,
                                 setting.fConditions));
  return;
}

void MTRBooster::trendsWrapper(MTRPlotSettings &setting, TMultiGraph* buffer)
{
//  std::cout<<"START\n";
  buffer = fShuttle.drawTrends(setting.funcY,
                               setting.fNormalize,
                               setting.fAccumulate,
                               setting.fPlotaverage,
                               setting.fPlane,
                               setting.fSide,
                               setting.fConditions);
}

void MTRBooster::minmaxWrapper(MTRPlotSettings &setting, TMultiGraph* buffer)
{
//  std::cout<<"START\n";
  buffer = fShuttle.drawMaxMin(setting.funcY,
                               setting.fNormalize,
                               setting.fAccumulate,
                               setting.fPlotaverage,
                               setting.fPlane,
                               setting.fSide,
                               setting.fConditions);
}
