//
// Created by Gabriele Gaetano Fronzé on 01/02/2018.
//

#include <iostream>
#include <fstream>
#include <ctime>
#include "AliCDBManager.h"
#include "AliGRPObject.h"
#include "AliCDBStorage.h"
#include "AliMpCDB.h"
#include "AliCDBEntry.h"
#include "AliMpDDLStore.h"
#include "AliMUONTriggerScalers.h"
#include <AliDCSValue.h>
#include <TRegexp.h>
#include "MTRShuttle.h"
#include "AlienUtils.h"
#include "VectorChecker.h"

ClassImp(MTRShuttle)

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
    auto position = std::find_if(fRunList.begin(),fRunList.end(),[&runBuffer](const std::pair<int,int>& pair){
      return pair.first == runBuffer;
    });
    if( position == fRunList.end() )fRunList.emplace_back(std::make_pair(runBuffer,AlienUtils::getRunYear(runBuffer)));
    if(fin.eof()) break;
  }
  fin.close();

  auto nOfRuns = fRunList.size();

  for (int plane=MTRPlanes::kMT11; plane<MTRPlanes::kNPlanes; plane++) {
    for (int side=kINSIDE; side<MTRSides::kNSides; side++) {
      for (int RPC=k1; RPC<MTRRPCs::kNRPCs; RPC++) {
        fRunDataVect[plane][side][RPC].reserve(nOfRuns);
      }
    }
  }
}

void MTRShuttle::parseOCDB(std::string path)
{
  AlienUtils::connectIfNeeded(path);

  AliCDBManager *managerCDB = AliCDBManager::Instance();

  int previousRunYear = 0;
  std::string CDBpath;

  for (const auto &runIterator : fRunList) {

    if(previousRunYear!=runIterator.second){
      previousRunYear=runIterator.second;
      CDBpath = path;
      CDBpath.replace(CDBpath.find("####"), 4,std::to_string(runIterator.second).c_str());
      managerCDB->SetDefaultStorage(CDBpath.c_str());
    }

    printf("\t\tINFO: Processing run %d\n",runIterator.first);

    int RunYear = runIterator.second;

    AliCDBStorage *defStorage = managerCDB->GetDefaultStorage();
    if (!defStorage) continue;

    managerCDB->SetRun(runIterator.first);
    defStorage->QueryCDB(runIterator.first);

    if (!AlienUtils::checkCDB(runIterator.first,defStorage,"GRP/GRP/Data",false)) {
      printf("\t\tERROR: GRP/GRP/Data not found for run %d\n",runIterator.first);
      continue;
    } else {
      printf("\t\tINFO: GRP/GRP/Data found for run %d\n",runIterator.first);
    }

    if (!AliMpCDB::LoadDDLStore()) continue;
    AliMpDDLStore *ddlStore = AliMpDDLStore::Instance();

    //inizializzazione dell'entry contente il runtype
    AliCDBEntry *entryRunType = managerCDB->Get("GRP/GRP/Data");
    if (!entryRunType) {
      printf("\t\tERROR: AliCDBEntry not found for run %d\n",runIterator.first);
      continue;
    }

    //retrievering delle informazioni sul run
    auto *grpObj = (AliGRPObject *) entryRunType->GetObject();
    if (!grpObj) {
      printf("\t\tERROR: AliGRPObject not found for run %d\n",runIterator.first);
      continue;
    }

    TString runType = grpObj->GetRunType();
    TString beamType = grpObj->GetBeamType();
    float beamEnergy = grpObj->GetBeamEnergy();
    TString LHCState = grpObj->GetLHCState();
    auto SOR = (uint64_t) grpObj->GetTimeStart();
    auto EOR = (uint64_t) grpObj->GetTimeEnd();

    //settaggio del flag beamPresence
    bool isBeamPresent = (beamEnergy > .1);

    if (!AlienUtils::checkCDB(runIterator.first,defStorage,"MUON/Calib/TriggerDCS",false)) {
      printf("\t\tERROR: TriggerDCS not found for run %d\n",runIterator.first);
      continue;
    } else {
      printf("\t\tINFO: TriggerDCS found for run %d\n",runIterator.first);
    }

    //inizializzazione dell'entry contente i valori di corrente
    AliCDBEntry *entryDCS = managerCDB->Get("MUON/Calib/TriggerDCS");
    if (!entryDCS) {
      printf("\t\tERROR: AliCDBEntry not found for run %d\n",runIterator.first);
      continue;
    }

    //mappa delle correnti
    auto mapDCS = (TMap *) entryDCS->GetObject();
    if (!mapDCS) {
      printf("\t\tERROR: mapDCS not found for run %d\n",runIterator.first);
      continue;
    }

    RunObject runObjectBuffer[MTRPlanes::kNPlanes][MTRSides::kNSides][MTRRPCs::kNRPCs];

    int badHVCounter = 0;

    //loop sui piani, i lati (inside e outside) e le RPC (9 per side)
    for (int plane=MTRPlanes::kMT11; plane<MTRPlanes::kNPlanes; plane++) {
      for (int side=kINSIDE; side<MTRSides::kNSides; side++) {
        for (int RPC=k1; RPC<MTRRPCs::kNRPCs; RPC++) {

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
          bool isHVOk = true;

          //loop sulle entry del vettore di misure di tensione
          for (int arrayIndex = 0; arrayIndex < (dataArrayVoltage->GetEntries()); arrayIndex++) {
            auto *value = (AliDCSValue *) dataArrayVoltage->At(arrayIndex);

            auto HV = value->GetFloat();

            bool HVcheck = HV > kMinWorkHV;
            isHVOk &= HVcheck;


            if (!HVcheck) {
              printf("HV not OK for %d %d %d %f\n",plane,side,RPC,HV);
              badHVCounter++;
              break;
            } else {
              avgHV += HV;
              counterHV++;
            }
          }

          runObjectBuffer[plane][side][RPC].setfIsHVOk(isHVOk);
          runObjectBuffer[plane][side][RPC].setfIsDark(!isBeamPresent);
          runObjectBuffer[plane][side][RPC].setRunNumber((uint64_t)runIterator.first);
          runObjectBuffer[plane][side][RPC].setSOR(SOR);
          runObjectBuffer[plane][side][RPC].setEOR(EOR);
          runObjectBuffer[plane][side][RPC].setAvgHV((counterHV != 0 && isHVOk) ? avgHV / counterHV : 0.);
        }
      }
    }

    if(badHVCounter==MTRPlanes::kNPlanes*MTRSides::kNSides*MTRRPCs::kNRPCs) {
      printf("\t\tINFO: Run %d has low HV and is skipped\n",runIterator.first);
      continue;
    }

    // Skipping runs with HV under lower limits
//    if (!isHVOkGlobal) continue;
    if (AlienUtils::checkCDB(runIterator.first,defStorage,"MUON/Calib/TriggerScalers",false)) {
      //inizializzazone dell'entry contenente le letture degli scalers
      printf("\t\tINFO: TriggerScaler found for run %d\n",runIterator.first);

      AliCDBEntry *entryScalers = managerCDB->Get("MUON/Calib/TriggerScalers");
      if (!entryScalers) continue;

      //array delle letture
      auto *arrayScalers = (TClonesArray *) entryScalers->GetObject();
      if (!arrayScalers) continue;

      uint64_t elapsedTime[kNCathodes][MTRPlanes::kNPlanes][MTRSides::kNSides][MTRRPCs::kNRPCs];
      uint64_t scalers[kNCathodes][MTRPlanes::kNPlanes][MTRSides::kNSides][MTRRPCs::kNRPCs];

      for (int plane=MTRPlanes::kMT11; plane<MTRPlanes::kNPlanes; plane++) {
        for (int side=kINSIDE; side<MTRSides::kNSides; side++) {
          for (int RPC=k1; RPC<MTRRPCs::kNRPCs; RPC++) {
            for (int cathode = 0; cathode < kNCathodes; cathode++) {
              scalers[cathode][side][plane][RPC] = 0;
              elapsedTime[cathode][side][plane][RPC] = 0;
            }
          }
        }
      }

      //loop sulle entries, sui piani, i catodi (bending e non bending) e le Local Boards (234 per piano)
      AliMUONTriggerScalers *scalersData = nullptr;
      TIter next(arrayScalers);
      while ((scalersData = static_cast<AliMUONTriggerScalers *>(next()))) {
        int arrayScalersEntries = arrayScalers->GetEntries();

        for (int plane = 0; plane < kNPlanes; plane++) {
          for (int cathode = 0; cathode < kNCathodes; cathode++) {
            for (int localBoard = 0; localBoard < kNLocalBoards; localBoard++) {

              int iRPC017 = (ddlStore->GetDEfromLocalBoard(localBoard + 1, plane + 10)) % 100;
              int iRPC09 = kRPCIndexes[iRPC017] - 1;
              int iSide = kRPCSides[iRPC017];

              // se in overflow passo alla LB successiva
              if (scalersData->GetLocScalStripOver(cathode, plane, localBoard) > 0) {
                continue;
              }

              auto value = scalersData->GetLocScalStrip(cathode, plane, localBoard);

//              if (value > 0 && iRPC017 == 0) printf("%d %d %d %d %llu\n", plane, cathode, localBoard, iRPC09, value);

              scalers[cathode][iSide][plane][iRPC09] += scalersData->GetLocScalStrip(cathode, plane, localBoard);
              elapsedTime[cathode][iSide][plane][iRPC09] += scalersData->GetDeltaT();
            }
          }
        }
      }

      for (int plane=MTRPlanes::kMT11; plane<MTRPlanes::kNPlanes; plane++) {
        for (int side=kINSIDE; side<MTRSides::kNSides; side++) {
          for (int RPC=k1; RPC<MTRRPCs::kNRPCs; RPC++) {
            double values[2] = {0., 0.};
            for (int cathode = 0; cathode < kNCathodes; cathode++) {
              if (elapsedTime[cathode][side][plane][RPC] > 0) {
                values[cathode] =
                  (double) scalers[cathode][side][plane][RPC] / (double) elapsedTime[cathode][side][plane][RPC];
//                printf("%d %d %d %llu %llu %f\n", plane, side, RPC, scalers[cathode][side][plane][RPC],
//                       elapsedTime[cathode][side][plane][RPC], values[cathode]);

              }
            }
            runObjectBuffer[plane][side][RPC].setScalBending(values[0]);
            runObjectBuffer[plane][side][RPC].setScalNotBending(values[1]);
          }
        }
      }
    } else {
      printf("\t\tERROR: TriggerScalers not found for run %d\n",runIterator.first);
    }

    printf("\t\tINFO: Saving run %d\n",runIterator.first);

    for (int plane=MTRPlanes::kMT11; plane<MTRPlanes::kNPlanes; plane++) {
      for (int side=kINSIDE; side<MTRSides::kNSides; side++) {
        for (int RPC=k1; RPC<MTRRPCs::kNRPCs; RPC++) {
          fRunDataVect[plane][side][RPC].emplace_back(runObjectBuffer[plane][side][RPC]);
        }
      }
    }
  }

  for (int plane=MTRPlanes::kMT11; plane<MTRPlanes::kNPlanes; plane++) {
    for (int side=kINSIDE; side<MTRSides::kNSides; side++) {
      for (int RPC=k1; RPC<MTRRPCs::kNRPCs; RPC++) {
        std::sort(fRunDataVect[plane][side][RPC].begin(),
                  fRunDataVect[plane][side][RPC].end(),
                  [](const RunObject &a, const RunObject &b) -> bool {
                    return a.getRunNumber() < b.getRunNumber();
                  });
      }
    }
  }

  MTRShuttle::createDummyRuns();
}

void MTRShuttle::parseOCDBiMon(std::string path){

  AlienUtils::connectIfNeeded(path);

  AliCDBManager *managerCDB = AliCDBManager::Instance();

  int previousRunYear = 0;
  std::string CDBpath;

  for (const auto &runIterator : fRunList) {

    AliCDBStorage *defStorage = managerCDB->GetDefaultStorage();
    if (!defStorage) continue;

    managerCDB->SetRun(runIterator.first);
    defStorage->QueryCDB(runIterator.first);

    if (!AlienUtils::checkCDB(runIterator.first, defStorage, "MUON/Calib/TriggerDCS", false)) {
      printf("\t\tERROR: TriggerDCS not found for run %d\n", runIterator.first);
      continue;
    } else {
      printf("\t\tINFO: TriggerDCS found for run %d\n", runIterator.first);
    }

    //inizializzazione dell'entry contente i valori di corrente
    AliCDBEntry *entryDCS = managerCDB->Get("MUON/Calib/TriggerDCS");
    if (!entryDCS) {
      printf("\t\tERROR: AliCDBEntry not found for run %d\n", runIterator.first);
      continue;
    }

    //mappa delle correnti
    auto mapDCS = (TMap *) entryDCS->GetObject();
    if (!mapDCS) {
      printf("\t\tERROR: mapDCS not found for run %d\n", runIterator.first);
      continue;
    }

    //loop sui piani, i lati (inside e outside) e le RPC (9 per side)
    for (int plane = MTRPlanes::kMT11; plane < MTRPlanes::kNPlanes; plane++) {
      for (int side = kINSIDE; side < MTRSides::kNSides; side++) {
        for (int RPC = k1; RPC < MTRRPCs::kNRPCs; RPC++) {

          //creazione di un pointer all'elemento della mappa delle tensioni
          TObjArray *dataArrayVoltage;
          dataArrayVoltage = (TObjArray *) (mapDCS->GetValue(
            Form("MTR_%s_MT%d_RPC%d_HV.actual.iMon", kSides[side].c_str(), kPlanes[plane], RPC + 1)));

          if (!dataArrayVoltage) {
            printf(" Problems getting dataArrayCurrent\n");
            break;
          }

          //loop sulle entry del vettore di misure di tensione
          for (int arrayIndex = 0; arrayIndex < (dataArrayVoltage->GetEntries()); arrayIndex++) {
            auto *value = (AliDCSValue *) dataArrayVoltage->At(arrayIndex);

            value->GetTimeStamp();
            fAMANDACurrentsVect[plane][side][RPC].emplace_back(AMANDACurrent((uint64_t)value->GetTimeStamp(),value->GetFloat(),0.,false));
            std::cout<<Form("MTR_%s_MT%d_RPC%d ", kSides[side].c_str(), kPlanes[plane], RPC + 1)<<fAMANDACurrentsVect[plane][side][RPC].back()<<std::endl;
          }
        }
      }
    }
  }
}

void MTRShuttle::parseAMANDAiMon(std::string path)
{
  int linesCounter = 0;
  {
    double dummyTimeStamp = 0;
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
    AMANDACurrent bufferCurrent;
    if (fin.is_open()) {
      while (!fin.eof()) {
        getline(fin, line);
        if (line.empty()) continue;
        std::cout << linesCounter++ << "\r";
        const char *charbuffer = (char *) line.c_str();
        if (!charbuffer) continue;
        sscanf(charbuffer, "%lf;MTR_%c", &dummyTimeStamp, &InsideOutside);
        char pattern[200];
        sprintf(pattern, "%%lf;MTR_%sSIDE_MT%%d_RPC%%d_HV.actual.iMon;%%lf", (InsideOutside == 'I' ? "IN" : "OUT"));
        sscanf(charbuffer, pattern, &timeStamp, &MT, &RPC, &current);
        bufferCurrent.setTimeStamp((uint64_t)timeStamp);
        bufferCurrent.setITot(current);
        fAMANDACurrentsVect[mts[MT]][(InsideOutside == 'I' ? 0 : 1)][RPC - 1].emplace_back(bufferCurrent);
      }
      std::cout << std::endl;
      fin.close();
    } else std::cout << "Unable to open file";
  }

  for (int plane=MTRPlanes::kMT11; plane<MTRPlanes::kNPlanes; plane++) {
    for (int side=kINSIDE; side<MTRSides::kNSides; side++) {
      for (int RPC=k1; RPC<MTRRPCs::kNRPCs; RPC++) {
        std::sort(fAMANDACurrentsVect[plane][side][RPC].begin(),
                  fAMANDACurrentsVect[plane][side][RPC].end(),
                  [](const AMANDACurrent &a, const AMANDACurrent &b) -> bool {
                    return a.getTimeStamp() < b.getTimeStamp();
                  });
      }
    }
  }

  std::cout << "Loaded " << linesCounter << "AMANDA current values" << std::endl;
}

void MTRShuttle::parseAMANDAvMon(std::string path)
{
  int linesCounter = 0;
  {
    double dummyTimeStamp = 0;
    double timeStamp = 0;
    double voltage = 0.;
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
    AMANDAVoltage bufferVoltage;
    if (fin.is_open()) {
      while (!fin.eof()) {
        getline(fin, line);
        if (line.empty()) continue;
        std::cout << linesCounter++ << "\r";
        const char *charbuffer = (char *) line.c_str();
        if (!charbuffer) continue;
        sscanf(charbuffer, "%lf;MTR_%c", &dummyTimeStamp, &InsideOutside);
        char pattern[200];
        sprintf(pattern, "%%lf;MTR_%sSIDE_MT%%d_RPC%%d_HV.actual.vMon;%%lf", (InsideOutside == 'I' ? "IN" : "OUT"));
        sscanf(charbuffer, pattern, &timeStamp, &MT, &RPC, &voltage);
        bufferVoltage.setTimeStamp((uint64_t)timeStamp);
        bufferVoltage.setHV(voltage);
        fAMANDAVoltagesVect[mts[MT]][(InsideOutside == 'I' ? 0 : 1)][RPC - 1].emplace_back(bufferVoltage);
      }
      std::cout << std::endl;
      fin.close();
    } else std::cout << "Unable to open file";
  }

  for (int plane=MTRPlanes::kMT11; plane<MTRPlanes::kNPlanes; plane++) {
    for (int side=kINSIDE; side<MTRSides::kNSides; side++) {
      for (int RPC=k1; RPC<MTRRPCs::kNRPCs; RPC++) {
        std::sort(fAMANDAVoltagesVect[plane][side][RPC].begin(),
                  fAMANDAVoltagesVect[plane][side][RPC].end(),
                  [](const AMANDAVoltage &a, const AMANDAVoltage &b) -> bool {
                    return a.getTimeStamp() < b.getTimeStamp();
                  });
      }
    }
  }

  std::cout << "Loaded " << linesCounter << "AMANDA voltages values" << std::endl;
}

void MTRShuttle::loadReplacedRPCs(std::string path){
  int linesCounter = 0;
  std::string line;
  std::ifstream fin(path);
  if (fin.is_open()) {
    while (!fin.eof()) {
      getline(fin, line);
      if (line.empty()) continue;
      linesCounter++;
      std::cout << linesCounter << " " << line << "\n";
      fReplacedRPCs.emplace_back(ReplacedRPC(line));
    }
  } else {
    std::cout << "Problem with replacements file.\n";
  }
}

void MTRShuttle::createDummyRuns(){

  std::cout << "Creating dummy runs...\n";

  for (int plane=MTRPlanes::kMT11; plane<MTRPlanes::kNPlanes; plane++) {
    for (int side = kINSIDE; side < MTRSides::kNSides; side++) {
      for (int RPC = k1; RPC < MTRRPCs::kNRPCs; RPC++) {
        // Incremental counter to enumerate added dummy runs
        uint64_t runNumber = 0;

        // Sorting the vector of RunData
        std::sort(fRunDataVect[plane][side][RPC].begin(),
                  fRunDataVect[plane][side][RPC].end(),
                  [](const RunObject &a, const RunObject &b) -> bool {
                    return a.getSOR() < b.getSOR();
                  });

        // Getting first AMANDA TS from iMon and vMon vectors
        auto firstAMANDAiMonTS = (!(fAMANDACurrentsVect[plane][side][RPC].empty()))?fAMANDACurrentsVect[plane][side][RPC].begin()->fTimeStamp:std::numeric_limits<uint64_t>::max();
        auto firstAMANDAvMonTS = (!(fAMANDAVoltagesVect[plane][side][RPC].empty()))?fAMANDAVoltagesVect[plane][side][RPC].begin()->fTimeStamp:std::numeric_limits<uint64_t>::max();

        // Retrieving the lowest TS
        auto firstAMANDATS = (firstAMANDAiMonTS<firstAMANDAvMonTS)?firstAMANDAiMonTS:firstAMANDAvMonTS;

        // First run of the RunData vector
        auto runObjectIt = fRunDataVect[plane][side][RPC].begin();
        // Original last run of RunData vector
        auto runObjectEnd = fRunDataVect[plane][side][RPC].end();

        // Create a dummy run from minimum AMANDA TS to first SOR-1
        if(firstAMANDATS<runObjectIt->getSOR()-1) {
          fRunDataVect[plane][side][RPC].emplace_back(RunObject(firstAMANDATS-1,runObjectIt->getSOR()-1));
          fRunDataVect[plane][side][RPC].back().setRunNumber(runNumber);
          fRunDataVect[plane][side][RPC].back().setfIsDummy(true);
          runNumber++;
          printf("####################################\nCreated first run %llu from %llu to %llu.\n",runNumber-1,fRunDataVect[plane][side][RPC].back().getSOR(),fRunDataVect[plane][side][RPC].back().getEOR());
        }

        // Creating dummy runs between EOR(n) and SOR(n+1). end-1 is needed because n+1 is required.
        for (; runObjectIt < runObjectEnd-1; runObjectIt++) {

          printf("Run %llu from %llu to %llu.\n", runObjectIt->getRunNumber(), runObjectIt->getSOR(),
                 runObjectIt->getEOR());

          if( runObjectIt->getEOR() < (runObjectIt+1)->getSOR()){
            fRunDataVect[plane][side][RPC].emplace_back(RunObject(runObjectIt->getEOR()+1,(runObjectIt+1)->getSOR()-1));
            fRunDataVect[plane][side][RPC].back().setRunNumber(runNumber);
            fRunDataVect[plane][side][RPC].back().setfIsDummy(true);
            runNumber++;
            printf("Created run %llu from %llu to %llu.\n",runNumber-1,fRunDataVect[plane][side][RPC].back().getSOR(),fRunDataVect[plane][side][RPC].back().getEOR());
          }
        }

        // Getting last AMANDA TS from iMon and vMon vectors
        auto lastAMANDAiMonTS = (!(fAMANDACurrentsVect[plane][side][RPC].empty()))?fAMANDACurrentsVect[plane][side][RPC].back().fTimeStamp:0;
        auto lastAMANDAvMonTS = (!(fAMANDAVoltagesVect[plane][side][RPC].empty()))?fAMANDAVoltagesVect[plane][side][RPC].back().fTimeStamp:0;

        // Retrieving the highest TS
        auto lastAMANDATS = (lastAMANDAiMonTS<lastAMANDAvMonTS)?lastAMANDAvMonTS:lastAMANDAiMonTS;

        // Create a dummy run from last EOR+1 to maximum AMANDA TS
        if(lastAMANDATS>(runObjectEnd-1)->getEOR()+1) {
          fRunDataVect[plane][side][RPC].emplace_back(RunObject((runObjectEnd-1)->getEOR()+1,lastAMANDATS+1));
          fRunDataVect[plane][side][RPC].back().setRunNumber(runNumber);
          fRunDataVect[plane][side][RPC].back().setfIsDummy(true);
          runNumber++;
          printf("Created last run %llu from %llu to %llu.\n####################################\n",runNumber-1,fRunDataVect[plane][side][RPC].back().getSOR(),fRunDataVect[plane][side][RPC].back().getEOR());
        }

        // Re-sorting the whole vector of RunData to put the dummy runs at the right spot
        std::sort(fRunDataVect[plane][side][RPC].begin(),
                  fRunDataVect[plane][side][RPC].end(),
                  [](const RunObject &a, const RunObject &b) -> bool {
                    return a.getSOR() < b.getSOR();
                  });
      }
    }
  }
}

void MTRShuttle::propagateAMANDA(bool weightedAverage)
{
  struct validityInterval{
    uint64_t start;
    uint64_t stop;
    validityInterval(uint64_t sta, uint64_t sto){
      start = sta;
      stop = sto;
    }
  };

  for (int plane=MTRPlanes::kMT11; plane<MTRPlanes::kNPlanes; plane++) {
    for (int side=kINSIDE; side<MTRSides::kNSides; side++) {
      for (int RPC=k1; RPC<MTRRPCs::kNRPCs; RPC++) {

        printf("MT%d %s RPC%d...\n Setting isHvOk... \n",kPlanes[plane],kSides[side].c_str(),RPC);

        // Creating a vector of validity intervals for HV
        std::vector<validityInterval> isHvOkIntervals;
        for(auto isHvOkIt = fAMANDAVoltagesVect[plane][side][RPC].begin();
            isHvOkIt<fAMANDAVoltagesVect[plane][side][RPC].end()-1;
            isHvOkIt++){
          if( isHvOkIt->getHV() > 8000. ) { //TODO: this value should not be hardcoded!
            isHvOkIntervals.emplace_back(validityInterval(isHvOkIt->getTimeStamp(),(isHvOkIt+1)->getTimeStamp()));
          }
        }

        // The instance of the iterator is external to the loop to avoid always starting from the first element
        auto setHvOkIt= fAMANDACurrentsVect[plane][side][RPC].begin();

        // Setting isHvOk for all current measurements
        for(const auto &intervalIt : isHvOkIntervals){
          for(; setHvOkIt<fAMANDACurrentsVect[plane][side][RPC].end(); setHvOkIt++){
            if(setHvOkIt->getTimeStamp()<intervalIt.start) continue;
            else if(setHvOkIt->getTimeStamp()>intervalIt.stop) {
              setHvOkIt--;
              break;
            }
            else setHvOkIt->setIsHvOk(true);
          }
        }

        printf("Setting isDark... \n");

        // Creating a vector of dark periods
        std::vector<validityInterval> isDarkIntervals;
        for (const auto &runObjectIt: fRunDataVect[plane][side][RPC]) {
          if( runObjectIt.isDark() ) {
            isDarkIntervals.emplace_back(validityInterval(runObjectIt.getSOR(),runObjectIt.getEOR()));
          }
        }

        // The instance of the iterator is external to the loop to avoid always starting from the first element
        auto setIsDarkIt= fAMANDACurrentsVect[plane][side][RPC].begin();

        // Setting isDark for all current measurements
        for(const auto &intervalIt : isDarkIntervals){
          for(; setIsDarkIt<fAMANDACurrentsVect[plane][side][RPC].end(); setIsDarkIt++){
            if(setIsDarkIt->getTimeStamp()<intervalIt.start) continue;
            else if(setIsDarkIt->getTimeStamp()>intervalIt.stop) {
              setIsDarkIt--;
              break;
            }
            else setIsDarkIt->setIsDark(true);
          }
        }

        // Iterator to point to the last dark current reading
        auto lastDarkIt = fAMANDACurrentsVect[plane][side][RPC].begin();
        bool wasPrevDark = lastDarkIt->isDark();

        printf("Setting iDark... \n");

        // Loop over the current readings
        for (auto darkCurrentIt=fAMANDACurrentsVect[plane][side][RPC].begin()+1;
             darkCurrentIt!=fAMANDACurrentsVect[plane][side][RPC].end();
             darkCurrentIt++) {

          // If previous reading and current one are dark, update last dark
          if ( wasPrevDark && darkCurrentIt->isDark() ) lastDarkIt = darkCurrentIt;

            // If previous reading wasn't dark, while current one is, set dark currents
          else if ( !wasPrevDark && darkCurrentIt->isDark()) {

            // Computing the parameters for the "dumb interpolation"
            const double m = getM(*lastDarkIt,*darkCurrentIt);
            const double q = getQ(*lastDarkIt,*darkCurrentIt);
            const double TS0 = lastDarkIt->getTimeStamp();

            // Assigning dark current values from interpolation to the not-dark readings
            if ( lastDarkIt+1 < darkCurrentIt-1 ) {
              std::for_each(lastDarkIt+1,darkCurrentIt-1,[&m,&q,&TS0](AMANDACurrent &reading){
                if( !(reading.isDark()) )reading.setIDark(m*(reading.getTimeStamp()-TS0)+q);
//                std::cout<<reading.getIDark()<<std::endl;
              });
            }
          }

          wasPrevDark = darkCurrentIt->isDark();
        }

        printf("Setting voltage... \n");

        auto voltageIt= fAMANDAVoltagesVect[plane][side][RPC].begin();

        for (auto &runObjectIt: fRunDataVect[plane][side][RPC]) {

          // Load SOR and EOR values
          auto SOR = runObjectIt.getSOR();
          auto EOR = runObjectIt.getEOR();

          double hvCumulus = 0.;
          double totalT = 0.;
          int iCounter = 0;

          // Loop over the voltage readings
          for ( ; voltageIt!=fAMANDAVoltagesVect[plane][side][RPC].end()-1; voltageIt++) {

            auto TS = voltageIt->getTimeStamp();

            // If the timestamp is after the EOR break the loop (aka pass to the following run)
            if ( TS > EOR ) break;

            auto nextTS = (voltageIt+1)->getTimeStamp();

            // If the timestamp is before the SOR
            if ( TS < SOR ) {
              // If nextTS is before SOR skip
              if ( nextTS < SOR ) continue;
                // Else the current value has to be averaged from SOR to nextTS
              else TS = SOR;
            }

            // If nextTS is after EOR the current value has to be averaged from TS to EOR
            if( nextTS >= EOR ) nextTS = EOR;

            // Compute deltaT
            auto deltaT = nextTS - TS;

//            std::cout << "computing with deltaT=" << deltaT <<"\n";

            // Add current value to average numerator sum
            if(weightedAverage) {
              hvCumulus += voltageIt->getHV() * (double) deltaT;
            } else {
              hvCumulus += voltageIt->getHV();
            }

            // Now deltaT should always be equal to EOR-SOR
            totalT += deltaT;
            iCounter++;
          }

          std::cout << "run " << runObjectIt.getRunNumber() << " had " << iCounter << " available voltage readings.\n";

          // Compute average and assign values to run object
          if(weightedAverage) {
            runObjectIt.setAvgHV((totalT > 0) ? hvCumulus / totalT : runObjectIt.getAvgHV());
          } else {
            runObjectIt.setAvgHV((iCounter > 0) ? hvCumulus / (double) iCounter : runObjectIt.getAvgHV());
          }
        }

        auto currentIt= fAMANDACurrentsVect[plane][side][RPC].begin();

        printf("Integrating and averaging... \n");

        for (auto &runObjectIt: fRunDataVect[plane][side][RPC]) {

          // Load SOR and EOR values
          auto SOR = runObjectIt.getSOR();
          auto EOR = runObjectIt.getEOR();

          double iDarkCumulus = 0.;
          double iTotCumulus = 0.;
          double totalT = 0.;
          int iCounter = 0;

          double integratedCharge = 0;
//          uint64_t previousTS = currentIt->getTimeStamp();

          // Loop over the current readings
          for ( ; currentIt!=fAMANDACurrentsVect[plane][side][RPC].end()-1; currentIt++) {

            auto TS = currentIt->getTimeStamp();

            // If the timestamp is after the EOR break the loop (aka pass to the following run)
            if ( TS > EOR ) break;

            auto nextTS = (currentIt+1)->getTimeStamp();

            // If the timestamp is before the SOR
            if ( TS < SOR ) {
              // If nextTS is before SOR skip
              if ( nextTS < SOR ) continue;
                // Else the current value has to be averaged from SOR to nextTS
              else TS = SOR;
            }

            // If nextTS is after EOR the current value has to be averaged from TS to EOR
            if( nextTS >= EOR ) nextTS = EOR;

            // Compute deltaT
            auto deltaT = nextTS - TS;

//            std::cout << "computing with deltaT=" << deltaT <<"\n";

            // Integrate current is HV is at working point
            if(currentIt->isHvOk() || !(currentIt->hasBeenFlagged())) integratedCharge += currentIt->getINet() * (double) deltaT;

            // Add current value to average numerator sum
            if(weightedAverage) {
              iDarkCumulus += currentIt->getIDark() * (double) deltaT;
              iTotCumulus += currentIt->getITot() * (double) deltaT;
            } else {
              iDarkCumulus += currentIt->getIDark();
              iTotCumulus += currentIt->getITot();
            }

            // Now deltaT should always be equal to EOR-SOR
            totalT += deltaT;
            iCounter++;
          }

          std::cout << "run " << runObjectIt.getRunNumber() << " had " << iCounter << " available current readings.\n";

          // Compute average and assign values to run object
          if(weightedAverage) {
            runObjectIt.setAvgIDark((totalT > 0) ? iDarkCumulus / totalT : 0.);
            runObjectIt.setAvgITot((totalT > 0) ? iTotCumulus / totalT : 0.);
            runObjectIt.setIntCharge(integratedCharge);
          } else {
            runObjectIt.setAvgIDark((iCounter > 0) ? iDarkCumulus / (double) iCounter : 0.);
            runObjectIt.setAvgITot((iCounter > 0) ? iTotCumulus / (double) iCounter : 0.);
            runObjectIt.setIntCharge(integratedCharge);
          }
        }
      }
    }
  }
}


void MTRShuttle::computeAverage()
{
  auto nOfRuns = fRunDataVect[MTRPlanes::kMT12][MTRSides::kINSIDE][MTRRPCs::k1].size();

  for(int iRun = 0; iRun < (int)nOfRuns; iRun++){

    RunObject runData;
    runData.setSOR(fRunDataVect[MTRPlanes::kMT12][MTRSides::kINSIDE][MTRRPCs::k1][iRun].getSOR());
    runData.setEOR(fRunDataVect[MTRPlanes::kMT12][MTRSides::kINSIDE][MTRRPCs::k1][iRun].getEOR());
    runData.setRunNumber(fRunDataVect[MTRPlanes::kMT12][MTRSides::kINSIDE][MTRRPCs::k1][iRun].getRunNumber());
    runData.setfIsDark(fRunDataVect[MTRPlanes::kMT12][MTRSides::kINSIDE][MTRRPCs::k1][iRun].isDark());

    for (int plane=MTRPlanes::kMT11; plane<MTRPlanes::kNPlanes; plane++) {
      auto nOfRPC = MTRSides::kNSides*MTRRPCs::kNRPCs;
      fRunDataVectAvg[plane].reserve(nOfRuns);
      for (int side=kINSIDE; side<MTRSides::kNSides; side++) {
        for (int RPC=k1; RPC<MTRRPCs::kNRPCs; RPC++) {
          if(fRunDataVect[plane][side][RPC][iRun].getAvgHV()>kMinWorkHV) {
            runData = runData + fRunDataVect[plane][side][RPC][iRun];
          }
          else {
            nOfRPC--;
          }
        }
      }

      if(nOfRPC!=0) runData = runData/(double)nOfRPC;

      fRunDataVectAvg[plane].emplace_back(runData);
    }

    RunObject runDataTot;
    runDataTot.setSOR(fRunDataVect[MTRPlanes::kMT12][MTRSides::kINSIDE][MTRRPCs::k1][iRun].getSOR());
    runDataTot.setEOR(fRunDataVect[MTRPlanes::kMT12][MTRSides::kINSIDE][MTRRPCs::k1][iRun].getEOR());
    runDataTot.setRunNumber(fRunDataVect[MTRPlanes::kMT12][MTRSides::kINSIDE][MTRRPCs::k1][iRun].getRunNumber());
    runDataTot.setfIsDark(fRunDataVect[MTRPlanes::kMT12][MTRSides::kINSIDE][MTRRPCs::k1][iRun].isDark());

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

  for (int plane=MTRPlanes::kMT11; plane<MTRPlanes::kNPlanes; plane++) {
    for (int side=kINSIDE; side<MTRSides::kNSides; side++) {
      for (int RPC=k1; RPC<MTRRPCs::kNRPCs; RPC++) {
        for (const auto &dataIt : fRunDataVect[plane][side][RPC]) {
          outputFile << plane << ";" << side << ";" << RPC << ";" << dataIt << "\n";
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
    while (!fin.eof() )
    {
      getline (fin,line);
      if (line.empty()) continue;
      std::cout<<"Loaded lines: "<<linesCounter++<<"\r";
      runObjectBuffer = RunObject(line,plane,side,RPC);
      fRunDataVect[plane][side][RPC].emplace_back(runObjectBuffer);
    }
    std::cout<<std::endl;
    fin.close();
  }
  else std::cout << "Unable to open file";
}

void MTRShuttle::graphMaquillage(MTRPlanes plane, MTRRPCs RPC, TGraph *graph, bool isAvgGraph)
{
  graph->GetXaxis()->SetLabelSize(0.035);
  graph->GetYaxis()->SetLabelSize(0.035);
  if(!isAvgGraph){
    graph->SetLineColor(kColors[RPC]);
    graph->SetMarkerColor(kColors[RPC]);
    graph->SetMarkerStyle(kMarkers[plane]);
    graph->SetMarkerSize(0.1);
    graph->SetLineStyle((Style_t)(1));
    graph->SetLineWidth(1);
  } else {
    graph->SetLineColor(kBlack);
    graph->SetMarkerColor(kBlack);
    graph->SetMarkerStyle(20);
    graph->SetMarkerSize(0.15);
    graph->SetLineStyle((Style_t)(1));
    graph->SetLineWidth(2);
  }
}
