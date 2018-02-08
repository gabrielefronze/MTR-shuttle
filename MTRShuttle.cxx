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
      std::cout<<linesCounter++<<"\r";
      runObjectBuffer = RunObject(line,plane,side,RPC);
      fRunDataVect[plane][side][RPC].emplace_back(runObjectBuffer);
    }
    std::cout<<std::endl;
    fin.close();
  }
  else std::cout << "Unable to open file";
}

void MTRShuttle::graphMaquillage(int plane, int side, int RPC, TGraph *graph)
{
  graph->SetLineColor(kColors[RPC]);
  graph->SetMarkerColor(kColors[RPC]);
  graph->SetMarkerStyle(kMarkers[plane]);
  graph->SetMarkerSize(0.1);
  graph->SetLineStyle((Style_t)((side==0)?1:9));
}

template<typename XType, typename YType> TGraph *MTRShuttle::drawCorrelation(int plane,
                                                                             int side,
                                                                             int RPC,
                                                                             XType(RunObject::*getX)() const,
                                                                             YType(RunObject::*getY)() const,
                                                                             bool normalizeToAreaX,
                                                                             bool normalizeToAreaY,
                                                                             bool accumulate)
{
  auto *returnedGraph = new TGraph();
  returnedGraph->SetNameTitle(Form("%d_%d_%d",plane,side,RPC),Form("MT%d %s RPC:%d",kPlanes[plane],kSides[side].c_str(),RPC));
  graphMaquillage(plane,side,RPC,returnedGraph);

  int counter = 0;

  auto yCumulus = (YType)0;

  for( auto const &dataIt : fRunDataVect[plane][side][RPC]){

    XType x = (dataIt.*getX)();
    YType y = (dataIt.*getY)();
    if (normalizeToAreaX) x=x/kAreas[plane][side][RPC];
    if (normalizeToAreaY) y=y/kAreas[plane][side][RPC];
    if ( y==(YType)0 ) continue;
    returnedGraph->SetPoint(counter++,(double)x,(double)((accumulate)?(yCumulus+=y):y)); //TODO: normalize to area
  }

  return returnedGraph;
}

template<typename XType, typename YType> TMultiGraph *MTRShuttle::drawCorrelation(XType(RunObject::*getX)() const,
                                                                                  YType(RunObject::*getY)() const,
                                                                                  bool normalizeToAreaX,
                                                                                  bool normalizeToAreaY,
                                                                                  bool accumulate)
{
  auto *returnedMultiGraph = new TMultiGraph();

  for (int plane=0; plane<kNPlanes; plane++) {
    for (int side = 0; side < kNSides; side++) {
      for (int RPC = 0; RPC < kNRPC; RPC++) {
        returnedMultiGraph->Add(drawCorrelation(plane,side,RPC,getX,getY,normalizeToAreaX,normalizeToAreaY,accumulate));
      }
    }
  }

  return returnedMultiGraph;
}

template<typename YType> TGraph *MTRShuttle::drawTrend(int plane,
                                                       int side,
                                                       int RPC,
                                                       YType(RunObject::*getY)() const,
                                                       bool normalizeToArea,
                                                       bool accumulate)
{
  gStyle->SetTimeOffset(0);
  auto returnedGraph = drawCorrelation(plane,side,RPC,&RunObject::getSOR,getY,false,normalizeToArea,accumulate);
  returnedGraph->GetXaxis()->SetTimeDisplay(1);
  returnedGraph->GetXaxis()->SetTimeFormat("%d\/%m\/%Y");
  returnedGraph->GetXaxis()->SetLabelSize(0.03);
  return returnedGraph;
}

template<typename YType>TMultiGraph *MTRShuttle::drawTrend(YType(RunObject::*getY)() const,
                                                           bool normalizeToArea,
                                                           bool accumulate)
{
  auto *returnedMultiGraph = new TMultiGraph();

  for (int plane=0; plane<kNPlanes; plane++) {
    for (int side = 0; side < kNSides; side++) {
      for (int RPC = 0; RPC < kNRPC; RPC++) {
        returnedMultiGraph->Add(drawTrend(plane,side,RPC,getY,normalizeToArea,accumulate));
      }
    }
  }

  return returnedMultiGraph;
}

//bool MTRShuttle::computeAreas(std::string path)
//{
//  AliCDBManager *managerCDB = AliCDBManager::Instance();
//  managerCDB->SetDefaultStorage(path.c_str());
//
//  managerCDB->SetRun(0);
//
//  AliCDBStorage *defStorage = managerCDB->GetDefaultStorage();
//  if (!defStorage) return false;
//
//  if (!AliMpCDB::LoadDDLStore()) return false;
//  AliMpDDLStore *ddlStore = AliMpDDLStore::Instance();
//
//  for ( Int_t plane=0; plane<4; plane++) {
//    for (Int_t LB = 1; LB <= 234; LB++) {
//      int ElemID = AliMpDDLStore::Instance()->GetDEfromLocalBoard(LB, 10 + plane);
//      int RPC017 = ElemID % 100;
//      int RPC09 = kRPCIndexes[RPC017] - 1;
//      int Side = kRPCSides[RPC017];
//
//      const AliMpVSegmentation *seg =
//        AliMpSegmentation::Instance()->GetMpSegmentation(ElemID, AliMp::CathodType::kCath0);
//      double x1 = 0., x2 = 0., y1 = 0., y2 = 0.;
//      // loop over strips
//      for (Int_t ibitxy = 0; ibitxy < 16; ibitxy += 15) {
//        AliMpPad pad = seg->PadByLocation(LB, ibitxy, kFALSE);
//        if (!pad.IsValid()) continue;
//        if (ibitxy == 0) {
//          x1 = pad.GetPositionX() - pad.GetDimensionX();
//          y1 = pad.GetPositionY() - pad.GetDimensionY();
//        }
//        x2 = pad.GetPositionX() + pad.GetDimensionX();
//        y2 = pad.GetPositionY() + pad.GetDimensionY();
//      }
//
//      fAreas[plane][Side][RPC09] += (x2 - x1) * (y2 - y1);
//    }
//  }
//
//  return true;
//}
