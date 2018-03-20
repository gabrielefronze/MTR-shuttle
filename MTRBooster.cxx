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

  for (int iSetting = 0; iSetting < fPlotSettings.size(); ++iSetting) {
    mgVector.emplace_back(MTRBooster::Launch(iSetting));
  }

  return mgVector;
}

TMultiGraph *MTRBooster::Launch(size_t iLaunch)
{
  if( iLaunch >= fPlotSettings.size() ) return nullptr;
  else {
    fCurrentPlotSetting = fPlotSettings[iLaunch];
    if( fCurrentPlotSetting.fRPC>=0 && fCurrentPlotSetting.fSide>=0 && fCurrentPlotSetting.fPlane>=0 ){
      if( isTimestamp(fCurrentPlotSetting.funcX) ){
        //TODO: it's a trend!!!
        return nullptr;
      } else {
        //TODO: it's a correlation!!!
        return nullptr;
      }
    } else {
      if( isTimestamp(fCurrentPlotSetting.funcX) ){
        //TODO: multiple trends!!!
        return nullptr;
      } else {
        //TODO: multiple correlations!!!
        return nullptr;
      }
    }
  }
}

MTRBooster *MTRBooster::SetPlane(int HR_plane)
{
  switch (HR_plane){
    case 11 : fCurrentPlotSetting.fPlane=0;
    case 12 : fCurrentPlotSetting.fPlane=1;
    case 21 : fCurrentPlotSetting.fPlane=2;
    case 22 : fCurrentPlotSetting.fPlane=3;
    case 13 : fCurrentPlotSetting.fPlane=2;
    case 14 : fCurrentPlotSetting.fPlane=3;
    default: fCurrentPlotSetting.fPlane=-1;
  }

  if( fCurrentPlotSetting.fPlane==-1 ) std::cerr << "Plane ID not recognised. Plotting all planes.\n";

  return this;
}

MTRBooster *MTRBooster::SetSide(std::string HR_side)
{
  if( HR_side.find("out")!=std::string::npos ) fCurrentPlotSetting.fSide=1;
  else if( HR_side.find("in")!=std::string::npos ) fCurrentPlotSetting.fSide=0;
  else if( HR_side.find("OUT")!=std::string::npos ) fCurrentPlotSetting.fSide=1;
  else if( HR_side.find("IN")!=std::string::npos ) fCurrentPlotSetting.fSide=0;
  else fCurrentPlotSetting.fSide=-1;

  if( fCurrentPlotSetting.fSide==-1 ) std::cerr << "Side not recognised. Plotting all sides.\n";

  return this;
}

MTRBooster *MTRBooster::SetRPC(int HR_RPC)
{
  if( HR_RPC<=9 && HR_RPC>=1 ) fCurrentPlotSetting.fRPC=HR_RPC-1;
  else fCurrentPlotSetting.fRPC=-1;

  if( fCurrentPlotSetting.fRPC==-1 ) std::cerr << "Wrong RPC ID. Plotting all RPCs.\n";

  return this;
}

MTRBooster *MTRBooster::SetX(std::string xValues)
{
  setGetterFromString(xValues, fCurrentPlotSetting.funcX);
  return this;
}

MTRBooster *MTRBooster::SetY(std::string yValues)
{
  setGetterFromString(yValues, fCurrentPlotSetting.funcY);
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
  if( !fAverageComputed ) fShuttle.computeAverage();
  fCurrentPlotSetting.fPlotaverage=true;
  return this;
}

uint64_t MTRBooster::getTSFromString(std::string date)
{
  std::tm time;
  time.tm_hour=0;
  time.tm_min=0;
  time.tm_sec=0;
  int dummyYear=0;
  std::tm timeGMT;

  sscanf(date.c_str(), "%d/%d/%d",&(time.tm_mday),&(time.tm_mon),&dummyYear);

  time.tm_year=(dummyYear>2000)?dummyYear:dummyYear+2000;
  time.tm_year-=1900;
  time.tm_hour++;
  time.tm_mon--;

  return (uint64_t)std::mktime(&timeGMT);
}

void MTRBooster::setGetterFromString(std::string func, double (RunObject::*funcPtr)() const)
{
  if ( func == "IDark" ) funcPtr = &RunObject::getAvgIDark;
  else if ( func == "ITot" ) funcPtr = &RunObject::getAvgITot;
  else if ( func == "INet" ) funcPtr = &RunObject::getAvgINet;
  else if ( func == "HV" ) funcPtr = &RunObject::getAvgHV;
  else if ( func == "RateBend" ) funcPtr = &RunObject::getScalBending;
  else if ( func == "RateNotBend" ) funcPtr = &RunObject::getScalNotBending;
  else if ( func == "IntCharge" ) funcPtr = &RunObject::getIntCharge;
  else {
    funcPtr = nullptr;
    std::cerr << "Requested function specifier " << func << " has not been recognised as valid.\n";
  }
}
