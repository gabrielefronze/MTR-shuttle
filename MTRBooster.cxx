//
// Created by Gabriele Gaetano Fronzé on 19/03/2018.
//

#include "MTRBooster.h"
#include "Parameters.h"
#include "TString.h"
#include "TCanvas.h"
#include "TLegend.h"
#include "RunObject.h"
#include <utility>

//MTRBooster::MTRBooster() : fLoadedData(false), fAverageComputed(false)
//{
//  std::string inputData;
//  std::cout << "No input file path provided. Please provide a valid .csv file path below:\n";
//  std::cin >> inputData;
//  fShuttle.loadData(std::move(inputData));
//  fLoadedData = true;
//}

MTRBooster::MTRBooster(std::string runListPath, std::string AMANDAiMonPath, std::string AMANDAvMonPath,
                       std::string OCDBPath, bool weighted)
  : fLoadedData(false), fAverageComputed(false)
{
  fShuttle.instance(std::move(runListPath), std::move(AMANDAiMonPath), std::move(AMANDAvMonPath), std::move(OCDBPath),
                    weighted);
  fLoadedData = true;
}

//MTRBooster::MTRBooster(std::string inputData) : fLoadedData(false), fAverageComputed(false)
//{
//  fShuttle.loadData(std::move(inputData));
//  fLoadedData = true;
//}

MTRBooster::MTRBooster(MTRShuttle shuttle) : fLoadedData(false), fAverageComputed(false)
{
  fShuttle = std::move(shuttle);
  fLoadedData = true;
}

void MTRBooster::Launch()
{
  fPlots.reserve(fPlotSettings.size());

  for (size_t iSetting = 0; iSetting < fPlotSettings.size(); ++iSetting) {

    fPlots.emplace_back(new TMultiGraph(Form("%zu", iSetting), ""));
    MTRBooster::Launch(iSetting, fPlots[iSetting]);
  }
}

void MTRBooster::Launch(size_t iSetting, TMultiGraph* buffer)
{
  if (!fLoadedData) {
    std::cerr << "Input data not defined. Aborting.\n";
    return;
  }

  if (iSetting >= fPlotSettings.size()) {
    std::cerr << "Plot settings index out of range. Aborting.\n";
    return;
  } else {
    auto currentSetting = (fPlotSettings[iSetting]);

    if (!buffer)
      buffer = new TMultiGraph(Form("%zu", iSetting), "");

    if (!currentSetting.fValidSettings) {
      std::cerr << "Configuration " << iSetting
                << " is not valid! The setting will be removed and the graph set to 0x0.\n";
      fPlotSettings.erase(fPlotSettings.begin() + iSetting);
      delete buffer;
      buffer = nullptr;
      return;
    }

//    if (!fAverageComputed && currentSetting.fPlotaverage)
//      MTRBooster::loadAverage();

    if (currentSetting.fRPC != MTRRPCs::kAllRPCs) {
      if (currentSetting.isTrend) {
        trendWrapper(currentSetting, buffer);
      } else {
        correlationWrapper(currentSetting, buffer);
      }
    } else {
      if (currentSetting.isTrend) {
        if (currentSetting.isMinMax) {
          minmaxWrapper(currentSetting, buffer);
        } else
          trendsWrapper(currentSetting, buffer);
      } else {
        correlationsWrapper(currentSetting, buffer);
      }
    }

    return;
  }
}

void MTRBooster::AutoDraw(size_t iPlot, TVirtualPad* pad, bool drawLegend, Option_t* opt, Option_t* legOpt)
{
  auto requiredPlot = fPlots[iPlot];
  auto settings = fPlotSettings[iPlot];

  if (pad && requiredPlot) {
    pad->cd();

    TString graphTitle = "";
    if (settings.fPlane != MTRPlanes::kAll)
      graphTitle.Append(Form("MT%d ", kPlanes[settings.fPlane]));
    if (settings.fSide != MTRSides::kBoth)
      graphTitle.Append(Form("%s ", kSides[settings.fSide].c_str()));
    if (settings.fRPC != MTRRPCs::kAllRPCs)
      graphTitle.Append(Form("%d ", settings.fRPC + 1));

    if (settings.isTrend || settings.isMinMax) {
      graphTitle.Append(generateTitle(settings.funcY));
      graphTitle.Append(" vs time");
    } else {
      graphTitle.Append(generateTitle(settings.funcY));
      graphTitle.Append(" vs ");
      graphTitle.Append(generateTitle(settings.funcX));
    }

    requiredPlot->SetTitle(graphTitle);

    requiredPlot->Draw(opt);

    requiredPlot->GetXaxis()->SetLabelSize(0.027);
    requiredPlot->GetYaxis()->SetLabelSize(0.027);
    requiredPlot->GetYaxis()->SetTitleOffset(1.5);

    if (settings.isMinMax || settings.isTrend) {
      // This time offset is NEEDED to correctly display data from timestamp!
      gStyle->SetTimeOffset(0);
      requiredPlot->GetXaxis()->SetTimeDisplay(1);
      requiredPlot->GetXaxis()->SetTimeFormat("%d/%m/%y");
      requiredPlot->GetXaxis()->SetTitle("Date");
      requiredPlot->GetXaxis()->SetLabelSize(0.023);
    } else
      requiredPlot->GetHistogram()->GetXaxis()->SetTitle(generateLabel(settings.funcX, settings.fNormalize).c_str());

    requiredPlot->GetHistogram()->GetYaxis()->SetTitle(generateLabel(settings.funcY, settings.fNormalize).c_str());

    if (drawLegend) {
      TString legTitle = (settings.fPlane < MTRPlanes::kNPlanes) ? Form("MT%d", kPlanes[settings.fPlane]) : "";

      auto legend = pad->BuildLegend(0.125, 0.73, 0.38, 0.90, legTitle, legOpt);
      legend->SetDrawOption(legOpt);
      legend->SetName(legTitle);
      //      legend->SetFillStyle(0);
      auto nGraphs = requiredPlot->GetListOfGraphs()->GetEntries();
      if (settings.fPlotaverage)
        nGraphs--;
      if (nGraphs > 5) {
        if (nGraphs % 2 == 0)
          legend->SetNColumns(2);
        if (nGraphs % 3 == 0)
          legend->SetNColumns(3);
      }
    }
  }
}

void MTRBooster::PrintConfig() { printf("\n"); }

MTRBooster& MTRBooster::SetPlane(int HR_plane)
{
  switch (HR_plane) {
    case 1:
      fCurrentPlotSetting.fPlane = MTRPlanes::kMT11;
      break;
    case 2:
      fCurrentPlotSetting.fPlane = MTRPlanes::kMT12;
      break;
    case 3:
      fCurrentPlotSetting.fPlane = MTRPlanes::kMT21;
      break;
    case 4:
      fCurrentPlotSetting.fPlane = MTRPlanes::kMT22;
      break;
    case 11:
      fCurrentPlotSetting.fPlane = MTRPlanes::kMT11;
      break;
    case 12:
      fCurrentPlotSetting.fPlane = MTRPlanes::kMT12;
      break;
    case 21:
      fCurrentPlotSetting.fPlane = MTRPlanes::kMT21;
      break;
    case 22:
      fCurrentPlotSetting.fPlane = MTRPlanes::kMT22;
      break;
    case 13:
      fCurrentPlotSetting.fPlane = MTRPlanes::kMT21;
      break;
    case 14:
      fCurrentPlotSetting.fPlane = MTRPlanes::kMT22;
      break;
    default:
      fCurrentPlotSetting.fPlane = MTRPlanes::kAll;
      break;
  }

  if (fCurrentPlotSetting.fPlane == kAll) {
    std::cerr << "Plane ID not recognised. Plotting all planes.\n";
    std::cerr << "Recognised IDs are: {1,2,3,4} or {11,12,13,14} or {11,12,21,22}\n";
  }

  return *this;
}

MTRBooster& MTRBooster::SetSide(std::string HR_side)
{
  if (HR_side.find("out") != std::string::npos)
    fCurrentPlotSetting.fSide = MTRSides::kOUTSIDE;
  else if (HR_side.find("in") != std::string::npos)
    fCurrentPlotSetting.fSide = MTRSides::kINSIDE;
  else if (HR_side.find("OUT") != std::string::npos)
    fCurrentPlotSetting.fSide = MTRSides::kOUTSIDE;
  else if (HR_side.find("IN") != std::string::npos)
    fCurrentPlotSetting.fSide = MTRSides::kINSIDE;
  else
    fCurrentPlotSetting.fSide = MTRSides::kBoth;

  if (fCurrentPlotSetting.fSide == kBoth) {
    std::cerr << "Side not recognised. Plotting all sides.\n";
    std::cerr << "Available options are: {IN,OUT}[SIDE] or {in,out}[side]\n";
  }
  return *this;
}

MTRBooster& MTRBooster::SetRPC(int HR_RPC)
{
  if (HR_RPC <= MTRRPCs::kNRPCs && HR_RPC >= 1)
    fCurrentPlotSetting.fRPC = (MTRRPCs)(HR_RPC - 1);
  else
    fCurrentPlotSetting.fRPC = kAllRPCs;

  if (fCurrentPlotSetting.fRPC == kAllRPCs) {
    std::cerr << "Wrong RPC ID. Plotting all RPCs.\n";
    std::cerr << "Recognised IDs are between 1 and 9\n";
  }
  return *this;
}

MTRBooster& MTRBooster::SetX(std::string xValues)
{
  setXGetterFromString(std::move(xValues));
  return *this;
}

MTRBooster& MTRBooster::SetY(std::string yValues)
{
  setYGetterFromString(std::move(yValues));
  return *this;
}

MTRBooster& MTRBooster::SetTSRange(uint64_t minTS, uint64_t maxTS)
{
  fCurrentPlotSetting.fConditions.addCondition(&RunObject::isBetweenTimestamps, false, minTS, maxTS);
  return *this;
}

MTRBooster& MTRBooster::SetMinTS(uint64_t minTS)
{
  fCurrentPlotSetting.fConditions.addCondition(&RunObject::isAfter, false, minTS);
  return *this;
}

MTRBooster& MTRBooster::SetMaxTS(uint64_t maxTS)
{
  fCurrentPlotSetting.fConditions.addCondition(&RunObject::isBefore, false, maxTS);
  return *this;
}

MTRBooster& MTRBooster::SetDateRange(std::string minDay, std::string maxDay)
{
  return SetTSRange(getTSFromString(std::move(minDay)), getTSFromString(std::move(maxDay)));
}

MTRBooster& MTRBooster::SetMinDate(std::string minDay) { return SetMinTS(getTSFromString(std::move(minDay))); }

MTRBooster& MTRBooster::SetMaxDate(std::string maxDay) { return SetMaxTS(getTSFromString(std::move(maxDay))); }

MTRBooster& MTRBooster::SetRunRange(uint64_t minRun, uint64_t maxRun)
{
  fCurrentPlotSetting.fConditions.addCondition(&RunObject::isBetweenRuns, false, minRun, maxRun);
  return *this;
}

MTRBooster& MTRBooster::SetMinRun(uint64_t minRun)
{
  fCurrentPlotSetting.fConditions.addCondition(&RunObject::isAfterRun, false, minRun);
  return *this;
}

MTRBooster& MTRBooster::SetMaxRun(uint64_t maxRun)
{
  fCurrentPlotSetting.fConditions.addCondition(&RunObject::isBeforeRun, false, maxRun);
  return *this;
}

MTRBooster& MTRBooster::OnlyHVOkRuns()
{
  fCurrentPlotSetting.fConditions.addCondition(&RunObject::isHVOk, false, this->fCurrentPlotSetting.fPlane, this->fCurrentPlotSetting.fSide, this->fCurrentPlotSetting.fRPC);
  return *this;
}

MTRBooster& MTRBooster::OnlyWithBeamRuns()
{
  fCurrentPlotSetting.fConditions.addCondition(&RunObject::isDark, true);
  return *this;
}

MTRBooster& MTRBooster::OnlyNoBeamRuns()
{
  fCurrentPlotSetting.fConditions.addCondition(&RunObject::isDark, false);
  return *this;
}

MTRBooster& MTRBooster::OnlyDarkCurrentRuns()
{
  fCurrentPlotSetting.fConditions.addCondition(&RunObject::isValidForIDark, false, this->fCurrentPlotSetting.fPlane, this->fCurrentPlotSetting.fSide, this->fCurrentPlotSetting.fRPC);
  return *this;
}

MTRBooster& MTRBooster::OnlyIntegratedChargeRuns()
{
  fCurrentPlotSetting.fConditions.addCondition(&RunObject::isValidForIntCharge, false, this->fCurrentPlotSetting.fPlane, this->fCurrentPlotSetting.fSide, this->fCurrentPlotSetting.fRPC);
  return *this;
}

uint64_t MTRBooster::getTSFromString(std::string date)
{
  std::tm timeStruct{};
  std::tm timeStructGMT{};
  int dummyYear;

  sscanf(date.c_str(), "%d/%d/%d", &(timeStruct.tm_mday), &(timeStruct.tm_mon), &dummyYear);

  timeStruct.tm_year = (dummyYear > 2000) ? dummyYear : dummyYear + 2000;
  timeStruct.tm_year -= 1900; // First epoch year is 1900
  timeStruct.tm_hour++;       // CERN is at GMT+1
  timeStruct.tm_mon--;        // Epoch starts from month=0 which corresponds to January

  time_t timeGMT = timegm(&timeStruct);
  timeStructGMT = *(gmtime(&timeGMT));

  return (uint64_t)std::mktime(&timeStructGMT);
}

void MTRBooster::setXGetterFromString(std::string func)
{
  if (func == "IDark")
    fCurrentPlotSetting.funcX = &RunObject::getAvgIDark;
  else if (func == "ITot")
    fCurrentPlotSetting.funcX = &RunObject::getAvgITot;
  else if (func == "INet")
    fCurrentPlotSetting.funcX = &RunObject::getAvgINet;
  else if (func == "HV")
    fCurrentPlotSetting.funcX = &RunObject::getAvgHV;
  else if (func == "RateBend")
    fCurrentPlotSetting.funcX = &RunObject::getScalBending;
  else if (func == "RateNotBend")
    fCurrentPlotSetting.funcX = &RunObject::getScalNotBending;
  else if (func == "IntCharge")
    fCurrentPlotSetting.funcX = &RunObject::getIntCharge;
  else if (func == "Time") {
    fCurrentPlotSetting.funcX = nullptr;
    fCurrentPlotSetting.isTrend = true;
  } else {
    fCurrentPlotSetting.funcX = nullptr;
    fCurrentPlotSetting.fValidSettings = false;
    std::cerr << "Requested function specifier " << func
              << " has not been recognised as valid. Configuration will be skipped.\n";
    std::cerr << "Valid specifiers are: IDark, ITot, INet, HV, RateBend, RateNotBend, IntCharge, Time\n";
    return;
  }

  fCurrentPlotSetting.fValidSettings = true;
}

void MTRBooster::setYGetterFromString(std::string func)
{
  if (func == "IDark")
    fCurrentPlotSetting.funcY = &RunObject::getAvgIDark;
  else if (func == "ITot")
    fCurrentPlotSetting.funcY = &RunObject::getAvgITot;
  else if (func == "INet")
    fCurrentPlotSetting.funcY = &RunObject::getAvgINet;
  else if (func == "HV")
    fCurrentPlotSetting.funcY = &RunObject::getAvgHV;
  else if (func == "RateBend")
    fCurrentPlotSetting.funcY = &RunObject::getScalBending;
  else if (func == "RateNotBend")
    fCurrentPlotSetting.funcY = &RunObject::getScalNotBending;
  else if (func == "IntCharge")
    fCurrentPlotSetting.funcY = &RunObject::getIntCharge;
  else if (func == "Time") {
    std::cerr << "Time can only be used as X of a graph. The configuration is invalid and will be skipped.";
    fCurrentPlotSetting.fValidSettings = false;
  } else {
    fCurrentPlotSetting.funcY = nullptr;
    fCurrentPlotSetting.fValidSettings = false;
    std::cerr << "Requested function specifier " << func
              << " has not been recognised as valid. Configuration will be skipped.\n";
    std::cerr << "Valid specifiers are: IDark, ITot, INet, HV, RateBend, RateNotBend, IntCharge, Time\n";
    return;
  }

  fCurrentPlotSetting.fValidSettings = true;
}

void MTRBooster::LoadReplacedRPCs(std::string path)
{
  int linesCounter = 0;
  std::string line;
  std::ifstream fin(path);
  if (fin.is_open()) {
    while (!fin.eof()) {
      getline(fin, line);
      if (line.empty())
        continue;
      linesCounter++;
      std::cout << linesCounter << " " << line << "\n";
      fReplacedRPCs.emplace_back(ReplacedRPC(line));
    }
  } else {
    std::cout << "Problem with replacements file.\n";
  }
}

void MTRBooster::correlationWrapper(MTRPlotSettings& setting, TMultiGraph* buffer)
{
  buffer->Add(this->drawCorrelation(setting.funcX, setting.funcY, setting.fNormalize, setting.fNormalize,
                                    setting.fAccumulate, setting.fPlotaverage, setting.fPlane, setting.fSide,
                                    setting.fRPC, &(setting.fConditions)));
}

void MTRBooster::correlationsWrapper(MTRPlotSettings& setting, TMultiGraph* buffer)
{
  //
  buffer->Add(this->drawCorrelations(setting.funcX, setting.funcY, setting.fNormalize, setting.fNormalize,
                                     setting.fAccumulate, setting.fPlotaverage, setting.fPlane, setting.fSide,
                                     &(setting.fConditions)));
}

void MTRBooster::trendWrapper(MTRPlotSettings& setting, TMultiGraph* buffer)
{
  buffer->Add(this->drawTrend(setting.funcY, setting.fNormalize, setting.fAccumulate, setting.fPlotaverage,
                              setting.fPlane, setting.fSide, setting.fRPC, &(setting.fConditions)));
}

void MTRBooster::trendsWrapper(MTRPlotSettings& setting, TMultiGraph* buffer)
{
  //
  buffer->Add(this->drawTrends(setting.funcY, setting.fNormalize, setting.fAccumulate, setting.fPlotaverage,
                               setting.fPlane, setting.fSide, &(setting.fConditions)));
}

void MTRBooster::minmaxWrapper(MTRPlotSettings& setting, TMultiGraph* buffer)
{
  //
  buffer->Add(this->drawMaxMin(setting.funcY, setting.fNormalize, setting.fAccumulate, setting.fPlotaverage,
                               setting.fPlane, setting.fSide, &(setting.fConditions)));
}
