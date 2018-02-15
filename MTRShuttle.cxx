//
// Created by Gabriele Gaetano Fronzé on 01/02/2018.
//

#include <iostream>
#include <fstream>
#include "AliCDBManager.h"
#include "AliGRPObject.h"
#include "AliCDBStorage.h"
#include "AliMpCDB.h"
#include "AliCDBEntry.h"
#include "AliMpDDLStore.h"
#include "AliMUONTriggerScalers.h"
#include <AliDCSValue.h>
#include <TClonesArray.h>
#include <TAxis.h>
#include <TStyle.h>
#include <TH1F.h>
#include "MTRShuttle.h"
#include "AlienUtils.h"

void MTRShuttle::parseRunList(std::string path)
{
  std::ifstream fin(path);
  int runBuffer = 0;

  if(!fin.is_open()) {
    std::cout<<"File not found"<<std::endl<<std::flush;
    return;
  }

  while(true){
    runBuffer = 0;

    fin >> runBuffer;
    if(fin.eof()) break;
    fRunList.emplace_back(std::make_pair(runBuffer,2017));
  }
  fin.close();

  auto nOfRuns = fRunList.size();

  for (int plane=0; plane<kNPlanes; plane++) {
    for (int side=0; side<kNSides; side++) {
      for (int RPC=0; RPC<kNRPC; RPC++) {
        fRunDataVect[plane][side][RPC].reserve(nOfRuns);
      }
    }
  }
}

void MTRShuttle::parseOCDB(std::string path)
{

  if (path.find("alien") != std::string::npos) {
    if (!AlienUtils::checkAlienToken()) {
      std::string userName;
      std::cerr << "Alien token is not valid" << std::endl;
      std::cout << "Please enter your Alien username " << std::endl;
      std::cin >> userName;
      AlienUtils::initAlienToken(userName);
      if (!AlienUtils::checkAlienToken()) {
        std::cerr << "Alien token not valid even after reset. Proceed manually!" << std::endl;
        return;
      }
    }
  }

  AliCDBManager *managerCDB = AliCDBManager::Instance();
  managerCDB->SetDefaultStorage(path.c_str());

  for (const auto &runIterator : fRunList) {
    AliCDBManager *managerYearCheck = managerCDB;

    int RunYear = runIterator.second;

    managerCDB->SetRun(runIterator.first);

    AliCDBStorage *defStorage = managerCDB->GetDefaultStorage();
    if (!defStorage) continue;

    defStorage->QueryCDB(runIterator.first);
    TObjArray *arrCDBID = defStorage->GetQueryCDBList();
    if (!arrCDBID) continue;
    TIter nxt(arrCDBID);
    AliCDBId *cdbID = 0;
    bool hasGRP = false;
    while ((cdbID = (AliCDBId *) nxt())) {
      if (cdbID->GetPath() == "GRP/GRP/Data") {
        hasGRP = kTRUE;
        break;
      }
    }
    if (!hasGRP) {
      continue;
    }

    if (!AliMpCDB::LoadDDLStore()) continue;
    AliMpDDLStore *ddlStore = AliMpDDLStore::Instance();

    //inizializzazione dell'entry contente il runtype
    AliCDBEntry *entryRunType = managerCDB->Get("GRP/GRP/Data");
    if (!entryRunType) continue;

    //retrievering delle informazioni sul run
    auto *grpObj = (AliGRPObject *) entryRunType->GetObject();
    if (!grpObj) continue;

    TString runType = grpObj->GetRunType();
    TString beamType = grpObj->GetBeamType();
    float beamEnergy = grpObj->GetBeamEnergy();
    TString LHCState = grpObj->GetLHCState();
    auto SOR = (uint64_t) grpObj->GetTimeStart();
    auto EOR = (uint64_t) grpObj->GetTimeEnd();

    //settaggio del flag beamPresence
    bool isBeamPresent = (beamEnergy > 1.);

    //inizializzazione dell'entry contente i valori di corrente
    AliCDBEntry *entryDCS = managerCDB->Get("MUON/Calib/TriggerDCS");
    if (!entryDCS) continue;

    //mappa delle correnti
    auto mapDCS = (TMap *) entryDCS->GetObject();
    if (!mapDCS) continue;

    RunObject runObjectBuffer[kNPlanes][kNSides][kNRPC];

    //loop sui piani, i lati (inside e outside) e le RPC (9 per side)
    for (int plane = 0; plane < kNPlanes; plane++) {
      for (int side = 0; side < kNSides; side++) {
        for (int RPC = 0; RPC < kNRPC; RPC++) {

          //creazione di un pointer all'elemento della mappa delle tensioni
          TObjArray *dataArrayVoltage;
          dataArrayVoltage = (TObjArray *) (mapDCS->GetValue(
            Form("MTR_%s_MT%d_RPC%d_HV.vEff", kSides[side].c_str(), kPlanes[plane], RPC + 1)));

          if (!dataArrayVoltage) {
            printf(" Problems getting dataArrayVoltage\n");
            break;
          }

//          bool isVoltageOk=true;
          double avgHV = 0.;
          int counterHV = 0;

          //loop sulle entry del vettore di misure di tensione
          for (int arrayIndex = 0; arrayIndex < (dataArrayVoltage->GetEntries()); arrayIndex++) {
            auto *value = (AliDCSValue *) dataArrayVoltage->At(arrayIndex);

            auto HV = value->GetFloat();

            if (HV < kMinWorkHV) {
              break;
            } else {
              avgHV += HV;
              counterHV++;
            }
          }

          runObjectBuffer[plane][side][RPC].setfIsDark(!(isBeamPresent));
          runObjectBuffer[plane][side][RPC].setRunNumber((uint64_t)runIterator.first);
          runObjectBuffer[plane][side][RPC].setSOR(SOR);
          runObjectBuffer[plane][side][RPC].setEOR(EOR);
          runObjectBuffer[plane][side][RPC].setAvgHV((counterHV != 0) ? avgHV / counterHV : 0.);
        }
      }
    }

    //inizializzazone dell'entry contenente le letture degli scalers
    AliCDBEntry *entryScalers = managerCDB->Get("MUON/Calib/TriggerScalers");
    if (!entryScalers) continue;

    //array delle letture
    auto *arrayScalers = (TClonesArray *) entryScalers->GetObject();
    if (!arrayScalers) continue;

    uint64_t elapsedTime = 0;
    double scalers[kNCathodes][kNSides][kNPlanes][kNRPC];
    bool overflowLB[kNCathodes][kNSides][kNPlanes][kNRPC];

    for (int plane = 0; plane < kNPlanes; plane++) {
      for (int side = 0; side < kNSides; side++) {
        for (int RPC = 0; RPC < kNRPC; RPC++) {
          for (int cathode = 0; cathode < kNCathodes; cathode++) {
            scalers[cathode][side][plane][RPC] = 0.;
            overflowLB[cathode][side][plane][RPC] = false;
          }
        }
      }
    }

    //loop sulle entries, sui piani, i catodi (bending e non bending) e le Local Boards (234 per piano)
    AliMUONTriggerScalers *scalersData = nullptr;
    TIter next(arrayScalers);
    while ( (scalersData = static_cast<AliMUONTriggerScalers*>(next())) ) {
      Int_t arrayScalersEntries = arrayScalers->GetEntries();
      elapsedTime += scalersData->GetDeltaT();

      for (int plane = 0; plane < kNPlanes; plane++) {
        for (int cathode = 0; cathode < kNCathodes; cathode++) {
          for (int localBoard = 0; localBoard < kNLocalBoards; localBoard++) {

            int iRPC017 = (ddlStore->GetDEfromLocalBoard(localBoard + 1, plane + 10)) % 100;
            int iRPC09 = kRPCIndexes[iRPC017] - 1;
            int iSide = kRPCSides[iRPC017];

            // se in overflow passo alla LB successiva
            if (scalersData->GetLocScalStripOver(cathode, plane, localBoard) != 0.) {
              overflowLB[cathode][iSide][plane][iRPC09] |= true;
              continue;
            }

            scalers[cathode][iSide][plane][iRPC09] += scalersData->GetLocScalStrip(cathode, plane, localBoard);
          }
        }
      }
    }

    for (int plane=0; plane<kNPlanes; plane++) {
      for (int side=0; side<kNSides; side++) {
        for (int RPC=0; RPC<kNRPC; RPC++) {
          double values[2] = {0.,0.};
          for (int cathode=0; cathode<kNCathodes; cathode++) {
            values[cathode] = scalers[cathode][side][plane][RPC] / (elapsedTime);
          }
          runObjectBuffer[plane][side][RPC].setScalBending(values[0]);
          runObjectBuffer[plane][side][RPC].setScalNotBending(values[1]);

          fRunDataVect[plane][side][RPC].emplace_back(runObjectBuffer[plane][side][RPC]);
        }
      }
    }

    printf("scalers reading complete.\n");
  }

  for (int plane=0; plane<kNPlanes; plane++) {
    for (int side = 0; side < kNSides; side++) {
      for (int RPC = 0; RPC < kNRPC; RPC++) {
        std::sort(fRunDataVect[plane][side][RPC].begin(),
                  fRunDataVect[plane][side][RPC].end(),
                  [](const RunObject &a, const RunObject &b) -> bool {
                    return a.getRunNumber() < b.getRunNumber();
                  });
      }
    }
  }
}

void MTRShuttle::parseAMANDAiMon(std::string path)
{
  int linesCounter = 0;
  {
    uint64_t dummyTimeStamp = 0;
    double timeStamp = 0;
    double current = 0.;
    int MT = 0;
    int RPC = 0;
    char InsideOutside = 'I';
    
    int mts[23];
    mts[11]=0;
    mts[12]=1;
    mts[21]=2;
    mts[22]=3;

    std::string line;
    std::ifstream fin(path);
    AMANDACurrent bufferCurrent(0., 0., false);
    if (fin.is_open()) {
      while (!fin.eof()) {
        getline(fin, line);
        if (fin.eof()) break;
        std::cout << linesCounter++ << "\r";
        const char *charbuffer = (char *) line.c_str();
        if (!charbuffer) continue;
        sscanf(charbuffer, "%llu;MTR_%c", &dummyTimeStamp, &InsideOutside);
        char pattern[200];
        sprintf(pattern, "%%lf;MTR_%sSIDE_MT%%d_RPC%%d_HV.actual.iMon;%%lf", (InsideOutside == 'I' ? "IN" : "OUT"));
        sscanf(charbuffer, pattern, &timeStamp, &MT, &RPC, &current);
        bufferCurrent.setTimeStamp(static_cast<uint64_t>(timeStamp));
        bufferCurrent.setITot(current);
        fAMANDACurrentsVect[mts[MT]][(InsideOutside == 'I' ? 0 : 1)][RPC - 1].emplace_back(bufferCurrent);
      }
      std::cout << std::endl;
      fin.close();
    } else std::cout << "Unable to open file";
  }

  for (int plane=0; plane<kNPlanes; plane++) {
    for (int side = 0; side < kNSides; side++) {
      for (int RPC = 0; RPC < kNRPC; RPC++) {
        std::sort(fAMANDACurrentsVect[plane][side][RPC].begin(),
                  fAMANDACurrentsVect[plane][side][RPC].end(),
                  [](const AMANDACurrent &a, const AMANDACurrent &b) -> bool {
                    return a.getTimeStamp() < b.getTimeStamp();
                  });
      }
    }
  }

  std::cout << "Loaded " << linesCounter << "AMANDA values" << std::endl;
}

void MTRShuttle::propagateAMANDA()
{
  for (int plane=0; plane<kNPlanes; plane++) {
    for (int side = 0; side < kNSides; side++) {
      for (int RPC = 0; RPC < kNRPC; RPC++) {

        printf("MT%d %s RPC%d... Setting isDark... ",kPlanes[plane],kSides[side].c_str(),RPC);

        // The instance of the iterator is external to the loop to avoid always starting from the first element
        auto setIsDarkIt= fAMANDACurrentsVect[plane][side][RPC].begin();

        // Loop over run objects to retreve SOR and EOR values
        for (const auto &runObjectIt: fRunDataVect[plane][side][RPC]) {

          // If run is not dark skip
          if (!runObjectIt.isDark()) continue;

          // Load SOR and EOR values
          auto SOR = runObjectIt.getSOR();
          auto EOR = runObjectIt.getEOR();

          // Loop over the current readings
          for ( setIsDarkIt; setIsDarkIt!=fAMANDACurrentsVect[plane][side][RPC].end(); setIsDarkIt++) {

            auto TS = setIsDarkIt->getTimeStamp();

            // If the timestamp is before the SOR skip
            if ( TS < SOR ) continue;
            // If the timestamp is after the EOR break the loop (aka pass to the following run)
            else if ( TS > EOR ){
              if (setIsDarkIt!=fAMANDACurrentsVect[plane][side][RPC].begin()) setIsDarkIt--;
              break;
            // If SOR<TS<EOR then set IDark
            } else {
              setIsDarkIt->setIDark(setIsDarkIt->getITot());
              setIsDarkIt->setIsDark(true);
            }
          }
        }

        // Iterator to point to the last dark current reading
        auto lastDarkIt = fAMANDACurrentsVect[plane][side][RPC].begin();
        bool wasPrevDark = lastDarkIt->isDark();

        printf("Setting iDark... ");

        // Loop over the current readings
        for (auto darkCurrentIt=fAMANDACurrentsVect[plane][side][RPC].begin();
             darkCurrentIt!=fAMANDACurrentsVect[plane][side][RPC].end();
             darkCurrentIt++) {

          // If previous reading and current one are dark update last dark
          if ( wasPrevDark && darkCurrentIt->isDark() ) lastDarkIt = darkCurrentIt;

          // If previous reading wasn't dark, while current one yes, set dark currents
          else if ( !wasPrevDark && darkCurrentIt->isDark()) {

            // Computing the parameters for the "dumb interpolation"
            const double m = getM(*lastDarkIt,*darkCurrentIt);
            const double q = getQ(*lastDarkIt,*darkCurrentIt);
            const double TS0 = lastDarkIt->getTimeStamp();

            // Assigning dark current values from interpolation to the not-dark readings
            if ( lastDarkIt+1 >= darkCurrentIt-1 ) {
              std::for_each(lastDarkIt+1,darkCurrentIt-1,[&m,&q,&TS0](AMANDACurrent &reading){
                reading.setIDark(m*(reading.getTimeStamp()-TS0)+q);
              });
            }
          }

          wasPrevDark = darkCurrentIt->isDark();
        }

        auto currentIt= fAMANDACurrentsVect[plane][side][RPC].begin();

        printf("Interpolating and averaging...\n");

        for (auto &runObjectIt: fRunDataVect[plane][side][RPC]) {

          // Load SOR and EOR values
          auto SOR = runObjectIt.getSOR();
          auto EOR = runObjectIt.getEOR();

          double iDarkCumulus = 0.;
          double iDarkCounter = 0;

          double iTotCumulus = 0.;
          double iTotCounter = 0;

          double integratedCharge = 0;
          uint64_t previousTS = currentIt->getTimeStamp();

          // Loop over the current readings
          for ( currentIt; currentIt!=fAMANDACurrentsVect[plane][side][RPC].end(); currentIt++) {

            auto TS = currentIt->getTimeStamp();

            // If the timestamp is before the SOR skip
            if ( TS < SOR ) {
              previousTS=TS;
              continue;
            }
              // If the timestamp is after the EOR break the loop (aka pass to the following run)
            else if ( TS > EOR ){
//              if (currentIt!=fAMANDACurrentsVect[plane][side][RPC].begin()) currentIt--;
              break;
              // If SOR<TS<EOR then set IDark
            } else {
              iDarkCumulus+=currentIt->getIDark();
              iDarkCounter++;

              iTotCumulus+=currentIt->getITot();
              iTotCounter++;

              integratedCharge+=currentIt->getINet()*(currentIt->getTimeStamp()-previousTS);

              previousTS = TS;
            }
          }

          runObjectIt.setAvgIDark((iDarkCounter>0)?iDarkCumulus/(double)iDarkCounter:0.);
          runObjectIt.setAvgITot((iTotCounter>0)?iTotCumulus/(double)iTotCounter:0.);
          runObjectIt.setIntCharge(integratedCharge);
        }
      }
    }
  }
}


void MTRShuttle::computeAverage()
{
  auto nOfRuns = fRunDataVect[0][0][0].size();
  auto nOfRPC = kNSides*kNRPC;

  for(int iRun = 0; iRun < nOfRuns; iRun++){

    RunObject runData;
    runData.setSOR(fRunDataVect[0][0][0][iRun].getSOR());
    runData.setEOR(fRunDataVect[0][0][0][iRun].getEOR());
    runData.setRunNumber(fRunDataVect[0][0][0][iRun].getRunNumber());
    runData.setfIsDark(fRunDataVect[0][0][0][iRun].isDark());

    for (int plane=0; plane<kNPlanes; plane++) {
      fRunDataVectAvg[plane].reserve(nOfRuns);

      for (int side = 0; side < kNSides; side++) {
        for (int RPC = 0; RPC < kNRPC; RPC++) {
          runData = runData + fRunDataVect[plane][side][RPC][iRun];
        }
      }

      runData = runData/(double)nOfRPC;

      fRunDataVectAvg[plane].emplace_back(runData);
    }

    RunObject runDataTot;
    runDataTot.setSOR(fRunDataVect[0][0][0][iRun].getSOR());
    runDataTot.setEOR(fRunDataVect[0][0][0][iRun].getEOR());
    runDataTot.setRunNumber(fRunDataVect[0][0][0][iRun].getRunNumber());
    runDataTot.setfIsDark(fRunDataVect[0][0][0][iRun].isDark());

    fRunDataVectAvg[4].reserve(nOfRuns);

    for (int plane=0; plane<kNPlanes; plane++) {
      runDataTot = runDataTot + fRunDataVectAvg[plane][iRun];
    }

    runDataTot = runDataTot/(double)kNPlanes;

    fRunDataVectAvg[4].emplace_back(runDataTot);
  }
}

void MTRShuttle::saveData(std::string path)
{
  std::ofstream outputFile(path.c_str());

  for (int plane=0; plane<kNPlanes; plane++) {
    for (int side = 0; side < kNSides; side++) {
      for (int RPC = 0; RPC < kNRPC; RPC++) {
        for (const auto &dataIt : fRunDataVect[plane][side][RPC]) {
          outputFile << plane << ";" << side << ";" << RPC << ";" << dataIt << "\n";
//          std::cout << plane << ";" << side << ";" << RPC << ";" << dataIt << "\n";
        }
      }
    }
  }
  outputFile.close();
}

void MTRShuttle::loadData(std::string path)
{
  std::string line;
  std::ifstream fin (path);
  int linesCounter = 0;
  int plane,side,RPC;
  RunObject runObjectBuffer;

  if (fin.is_open())
  {
    while (! fin.eof() )
    {
      getline (fin,line);
      if (fin.eof()) break;
      std::cout<<"Loaded lines: "<<linesCounter++<<"\r";
      runObjectBuffer = RunObject(line,plane,side,RPC);
      fRunDataVect[plane][side][RPC].emplace_back(runObjectBuffer);
    }
    std::cout<<std::endl;
    fin.close();
  }
  else std::cout << "Unable to open file";
}

void MTRShuttle::graphMaquillage(int plane, int side, int RPC, TGraph *graph, bool isAvgGraph)
{
  if(!isAvgGraph){
    graph->SetLineColor(kColors[RPC]);
    graph->SetMarkerColor(kColors[RPC]);
    graph->SetMarkerStyle(kMarkers[plane]);
    graph->SetMarkerSize(0.1);
    graph->SetLineStyle((Style_t)(4));
  } else {
    graph->SetLineColor(kBlack);
    graph->SetMarkerColor(kBlack);
    graph->SetMarkerStyle(34);
    graph->SetLineStyle((Style_t)(1));
    graph->SetLineWidth(2);
  }
}

template<typename XType, typename YType>
TGraph *MTRShuttle::drawCorrelation(XType (RunObject::*getX)() const,
                                    YType (RunObject::*getY)() const,
                                    bool normalizeToAreaX,
                                    bool normalizeToAreaY,
                                    bool accumulate,
                                    bool plotAverage,
                                    int plane,
                                    int side,
                                    int RPC,
                                    bool (RunObject::*condition)() const,
                                    bool negateCondition)
{
  auto *returnedGraph = new TGraph();
  if(!plotAverage){
    if (plane<0) returnedGraph->SetNameTitle(Form("%d_%d_%d",plane,side,RPC+1),Form("MT%d %s %d",kPlanes[plane],kSidesShort[side].c_str(),RPC+1));
    else returnedGraph->SetNameTitle(Form("%d_%d",side,RPC+1),Form("%s %d",kSidesShort[side].c_str(),RPC+1));
  } else {
    returnedGraph->SetNameTitle("avg","Average");
  }
  graphMaquillage(plane,side,RPC,returnedGraph,plotAverage);

  returnedGraph->GetXaxis()->SetTitle(getLabel(getX,normalizeToAreaX).c_str());
  returnedGraph->GetYaxis()->SetTitle(getLabel(getY,normalizeToAreaY).c_str());

  if (compareFunctions(getX,&RunObject::getSOR) || compareFunctions(getX,&RunObject::getEOR)){
    //This time offset is NEEDED to correctly display data from timestamp!
    gStyle->SetTimeOffset(0);
    returnedGraph->GetXaxis()->SetTimeDisplay(1);
    returnedGraph->GetXaxis()->SetTimeFormat("%d-%m-%y");
    returnedGraph->GetXaxis()->SetLabelSize(0.02);
    returnedGraph->GetXaxis()->SetTitle("Date");
  }

  if (compareFunctions(getY,&RunObject::getSOR) || compareFunctions(getY,&RunObject::getEOR)){
    //This time offset is NEEDED to correctly display data from timestamp!
    gStyle->SetTimeOffset(0);
    returnedGraph->GetYaxis()->SetTimeDisplay(1);
    returnedGraph->GetYaxis()->SetTimeFormat("%d-%m-%y");
    returnedGraph->GetYaxis()->SetLabelSize(0.02);
    returnedGraph->GetYaxis()->SetTitle("Date");
  }

  int counter = 0;

  auto yCumulus = (YType)4000.;

  auto dataVector = (!plotAverage)?fRunDataVect[plane][side][RPC]:fRunDataVectAvg[(plane>=0)?plane:4];

  bool resetMT12OUTSIDE6 = true;
  bool isReplacedRPC = ( plane==1
                       && side==1
                       && RPC==5
                       && compareFunctions(getY,&RunObject::getIntCharge)
                       && accumulate );

  for( auto const &dataIt : dataVector){

    if ((dataIt.*condition)()==negateCondition) continue;

    XType x = (dataIt.*getX)();
    YType y = (dataIt.*getY)();

    if (normalizeToAreaX){
      if(!plotAverage) x=x/kAreas[plane][side][RPC];
      else x=x/kAreas[0][0][0];
    }

    if (normalizeToAreaY){
      if(!plotAverage) y=y/kAreas[plane][side][RPC];
      else y=y/kAreas[0][0][0];
    }

    if ( y==(YType)0 ) continue;

    if ( isReplacedRPC && resetMT12OUTSIDE6 && dataIt.getSOR()>1477958400 ){
      resetMT12OUTSIDE6 = false;
      yCumulus = (YType)0;
    }
    returnedGraph->SetPoint(counter++,(double)x,(double)((accumulate)?(yCumulus+=y):y));
  }

  return returnedGraph;
}

template<typename XType, typename YType> TMultiGraph *MTRShuttle::drawCorrelations(XType(RunObject::*getX)() const,
                                                                                   YType(RunObject::*getY)() const,
                                                                                   bool normalizeToAreaX,
                                                                                   bool normalizeToAreaY,
                                                                                   bool accumulate,
                                                                                   bool plotAverage,
                                                                                   int MT,
                                                                                   bool (RunObject::*condition)() const,
                                                                                   bool negateCondition)
{
  auto *mg = new TMultiGraph();

  for (int plane=0; plane<kNPlanes; plane++) {
    if ( MT>=0 && plane!=MT ) continue;
    for (int side = 0; side < kNSides; side++) {
      for (int RPC = 0; RPC < kNRPC; RPC++) {
        mg->Add(
          drawCorrelation(getX,
                          getY,
                          normalizeToAreaX,
                          normalizeToAreaY,
                          accumulate,
                          false,
                          plane,
                          side,
                          RPC,
                          condition,
                          negateCondition));
      }
    }
  }

  if (plotAverage) {
    mg->Add(
      drawCorrelation(getX,
                      getY,
                      normalizeToAreaX,
                      normalizeToAreaY,
                      accumulate,
                      plotAverage,
                      MT,
                      -1,
                      -1,
                      condition,
                      negateCondition));
  }

  mg->Draw("ap");
  mg->GetHistogram()->GetXaxis()->SetTimeOffset(0);
  mg->GetHistogram()->GetXaxis()->SetTimeDisplay(1);
  mg->GetHistogram()->GetXaxis()->SetTimeFormat("%d-%m-%y");
  mg->GetHistogram()->GetYaxis()->SetLabelSize(0.02);

  mg->GetHistogram()->GetXaxis()->SetTitle("Date");
  mg->GetHistogram()->GetYaxis()->SetTitle(getLabel(getY,normalizeToAreaY).c_str());

  return mg;
}

template<typename YType>
TGraph *MTRShuttle::drawTrend(YType (RunObject::*getY)() const,
                              bool normalizeToArea,
                              bool accumulate,
                              bool plotAverage,
                              int plane,
                              int side,
                              int RPC,
                              bool (RunObject::*condition)() const,
                              bool negateCondition)
{
  return drawCorrelation(&RunObject::getSOR,
                         getY,
                         false,
                         normalizeToArea,
                         accumulate,
                         plotAverage,
                         plane,
                         side,
                         RPC,
                         condition,
                         negateCondition);
}

template<typename YType>
TMultiGraph *MTRShuttle::drawTrends(YType (RunObject::*getY)() const,
                                    bool normalizeToArea,
                                    bool accumulate,
                                    bool plotAverage,
                                    int plane,
                                    bool (RunObject::*condition)() const,
                                    bool negateCondition)
{
  return drawCorrelations(&RunObject::getSOR,
                          getY,
                          false,
                          normalizeToArea,
                          accumulate,
                          plotAverage,
                          plane,
                          condition,
                          negateCondition);
}

template<typename YType>
TMultiGraph *
MTRShuttle::drawMaxMin(YType (RunObject::*getY)() const,
                       bool normalizeToAreaY,
                       bool accumulate,
                       bool plotAverage,
                       int MT,
                       bool (RunObject::*condition)() const,
                       bool negateCondition)
{
  auto mg = drawTrends(getY,normalizeToAreaY,accumulate,plotAverage,MT,condition,negateCondition);
  auto grList = mg->GetListOfGraphs();

  auto *mgOut = new TMultiGraph();

  if(plotAverage){
    auto avgGraph = (TGraph*)grList->FindObject("avg");
    if (avgGraph) mgOut->Add(avgGraph);
  }

  TGraph *minGraph = nullptr;
  TGraph *maxGraph = nullptr;

  Double_t minValue=1e+19;
  Double_t maxValue=0.;

  for(int iGraph = 0; iGraph < grList->GetEntries(); iGraph++){
    auto graph = (TGraph*)(grList->At(iGraph));

    Double_t dummyX;
    Double_t dummyY;

    graph->GetPoint(graph->GetN()-1, dummyX, dummyY);

    if (iGraph==0) {
      minValue = dummyY;
      minGraph = graph;

      maxValue = dummyY;
      maxGraph = graph;
    } else {
      if ( dummyY < minValue ){
        minValue = dummyY;
        minGraph = graph;
      }

      if ( dummyY > maxValue ){
        maxValue = dummyY;
        maxGraph = graph;
      }
    }
  }

  if (minGraph) mgOut->Add(minGraph);
  if (maxGraph) mgOut->Add(maxGraph);

  mgOut->Draw("ap");
  mgOut->GetHistogram()->GetXaxis()->SetTimeOffset(0);
  mgOut->GetHistogram()->GetXaxis()->SetTimeDisplay(1);
  mgOut->GetHistogram()->GetXaxis()->SetTimeFormat("%d-%m-%y");
  mgOut->GetHistogram()->GetYaxis()->SetLabelSize(0.02);

  mgOut->GetHistogram()->GetXaxis()->SetTitle("Date");
  mgOut->GetHistogram()->GetYaxis()->SetTitle(getLabel(getY,normalizeToAreaY).c_str());

  return mgOut;
}
