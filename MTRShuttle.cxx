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

  if (!fin.is_open()) {
    std::cout << "File not found" << std::endl << std::flush;
    return;
  }

  while (true) {
    runBuffer = 0;

    fin >> runBuffer;
    auto position = std::find_if(fRunList.begin(), fRunList.end(),
                                 [&runBuffer](const std::pair<int, int>& pair) { return pair.first == runBuffer; });
    if (position == fRunList.end())
      fRunList.emplace_back(std::make_pair(runBuffer, AlienUtils::getRunYear(runBuffer)));
    if (fin.eof())
      break;
  }
  fin.close();

  auto nOfRuns = fRunList.size();

  for (int plane = MTRPlanes::kMT11; plane < MTRPlanes::kNPlanes; plane++) {
    for (int side = kINSIDE; side < MTRSides::kNSides; side++) {
      for (int RPC = k1; RPC < MTRRPCs::kNRPCs; RPC++) {
        fRunDataVect[plane][side][RPC].reserve(nOfRuns);
      }
    }
  }
}

void MTRShuttle::parseOCDB(std::string path)
{
  AlienUtils::connectIfNeeded(path);

  AliCDBManager* managerCDB = AliCDBManager::Instance();

  int previousRunYear = 0;
  std::string CDBpath;

  for (const auto& runIterator : fRunList) {

    if (previousRunYear != runIterator.second) {
      previousRunYear = runIterator.second;
      CDBpath = path;
      CDBpath.replace(CDBpath.find("####"), 4, std::to_string(runIterator.second).c_str());
      managerCDB->SetDefaultStorage(CDBpath.c_str());
    }

    printf("\t\tINFO: Processing run %d\n", runIterator.first);

    int RunYear = runIterator.second;

    AliCDBStorage* defStorage = managerCDB->GetDefaultStorage();
    if (!defStorage)
      continue;

    managerCDB->SetRun(runIterator.first);
    defStorage->QueryCDB(runIterator.first);

    if (!AlienUtils::checkCDB(runIterator.first, defStorage, "GRP/GRP/Data", false)) {
      printf("\t\tERROR: GRP/GRP/Data not found for run %d\n", runIterator.first);
      continue;
    } else {
      printf("\t\tINFO: GRP/GRP/Data found for run %d\n", runIterator.first);
    }

    if (!AliMpCDB::LoadDDLStore())
      continue;
    AliMpDDLStore* ddlStore = AliMpDDLStore::Instance();

    // inizializzazione dell'entry contente il runtype
    AliCDBEntry* entryRunType = managerCDB->Get("GRP/GRP/Data");
    if (!entryRunType) {
      printf("\t\tERROR: AliCDBEntry not found for run %d\n", runIterator.first);
      continue;
    }

    // retrievering delle informazioni sul run
    auto* grpObj = (AliGRPObject*)entryRunType->GetObject();
    if (!grpObj) {
      printf("\t\tERROR: AliGRPObject not found for run %d\n", runIterator.first);
      continue;
    }

    TString runType = grpObj->GetRunType();
    TString beamType = grpObj->GetBeamType();
    float beamEnergy = grpObj->GetBeamEnergy();
    TString LHCState = grpObj->GetLHCState();
    auto SOR = (uint64_t)grpObj->GetTimeStart();
    auto EOR = (uint64_t)grpObj->GetTimeEnd();

    // settaggio del flag beamPresence
    bool isBeamPresent = (beamEnergy > 100.);

    if (!AlienUtils::checkCDB(runIterator.first, defStorage, "MUON/Calib/TriggerDCS", false)) {
      printf("\t\tERROR: TriggerDCS not found for run %d\n", runIterator.first);
      continue;
    } else {
      printf("\t\tINFO: TriggerDCS found for run %d\n", runIterator.first);
    }

    // inizializzazione dell'entry contente i valori di corrente
    AliCDBEntry* entryDCS = managerCDB->Get("MUON/Calib/TriggerDCS");
    if (!entryDCS) {
      printf("\t\tERROR: AliCDBEntry not found for run %d\n", runIterator.first);
      continue;
    }

    // mappa delle correnti
    auto mapDCS = (TMap*)entryDCS->GetObject();
    if (!mapDCS) {
      printf("\t\tERROR: mapDCS not found for run %d\n", runIterator.first);
      continue;
    }

    RunObject runObjectBuffer[MTRPlanes::kNPlanes][MTRSides::kNSides][MTRRPCs::kNRPCs];

    int badHVCounter = 0;

    // loop sui piani, i lati (inside e outside) e le RPC (9 per side)
    for (int plane = MTRPlanes::kMT11; plane < MTRPlanes::kNPlanes; plane++) {
      for (int side = kINSIDE; side < MTRSides::kNSides; side++) {
        for (int RPC = k1; RPC < MTRRPCs::kNRPCs; RPC++) {

          // creazione di un pointer all'elemento della mappa delle tensioni
          TObjArray* dataArrayVoltage;
          dataArrayVoltage = (TObjArray*)(mapDCS->GetValue(
            Form("MTR_%s_MT%d_RPC%d_HV.vEff", kSides[side].c_str(), kPlanes[plane], RPC + 1)));

          if (!dataArrayVoltage) {
            printf(" Problems getting dataArrayVoltage\n");
            break;
          }

          //          bool isVoltageOk=true;
          double avgHV = 0.;
          int counterHV = 0;
          bool isHVOk = true;

          // loop sulle entry del vettore di misure di tensione
          for (int arrayIndex = 0; arrayIndex < (dataArrayVoltage->GetEntries()); arrayIndex++) {
            auto* value = (AliDCSValue*)dataArrayVoltage->At(arrayIndex);

            auto HV = value->GetFloat();

            bool HVcheck = HV > kMinWorkHV;
            isHVOk &= HVcheck;

            if (!HVcheck) {
              printf("HV not OK for %d %d %d %f\n", plane, side, RPC, HV);
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

    if (badHVCounter == MTRPlanes::kNPlanes * MTRSides::kNSides * MTRRPCs::kNRPCs) {
      printf("\t\tINFO: Run %d has low HV and is skipped\n", runIterator.first);
      continue;
    }

    // Skipping runs with HV under lower limits
    //    if (!isHVOkGlobal) continue;
    if (AlienUtils::checkCDB(runIterator.first, defStorage, "MUON/Calib/TriggerScalers", false)) {
      // inizializzazone dell'entry contenente le letture degli scalers
      printf("\t\tINFO: TriggerScaler found for run %d\n", runIterator.first);

      AliCDBEntry* entryScalers = managerCDB->Get("MUON/Calib/TriggerScalers");
      if (!entryScalers)
        continue;

      // array delle letture
      auto* arrayScalers = (TClonesArray*)entryScalers->GetObject();
      if (!arrayScalers)
        continue;

      uint64_t elapsedTime[kNCathodes][MTRPlanes::kNPlanes][MTRSides::kNSides][MTRRPCs::kNRPCs];
      uint64_t scalers[kNCathodes][MTRPlanes::kNPlanes][MTRSides::kNSides][MTRRPCs::kNRPCs];

      for (int plane = MTRPlanes::kMT11; plane < MTRPlanes::kNPlanes; plane++) {
        for (int side = kINSIDE; side < MTRSides::kNSides; side++) {
          for (int RPC = k1; RPC < MTRRPCs::kNRPCs; RPC++) {
            for (int cathode = 0; cathode < kNCathodes; cathode++) {
              scalers[cathode][side][plane][RPC] = 0;
              elapsedTime[cathode][side][plane][RPC] = 0;
            }
          }
        }
      }

      // loop sulle entries, sui piani, i catodi (bending e non bending) e le Local Boards (234 per piano)
      AliMUONTriggerScalers* scalersData = nullptr;
      TIter next(arrayScalers);
      while ((scalersData = static_cast<AliMUONTriggerScalers*>(next()))) {
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

              //              if (value > 0 && iRPC017 == 0) printf("%d %d %d %d %llu\n", plane, cathode, localBoard,
              //              iRPC09, value);

              scalers[cathode][iSide][plane][iRPC09] += scalersData->GetLocScalStrip(cathode, plane, localBoard);
              elapsedTime[cathode][iSide][plane][iRPC09] += scalersData->GetDeltaT();
            }
          }
        }
      }

      for (int plane = MTRPlanes::kMT11; plane < MTRPlanes::kNPlanes; plane++) {
        for (int side = kINSIDE; side < MTRSides::kNSides; side++) {
          for (int RPC = k1; RPC < MTRRPCs::kNRPCs; RPC++) {
            double values[2] = { 0., 0. };
            for (int cathode = 0; cathode < kNCathodes; cathode++) {
              if (elapsedTime[cathode][side][plane][RPC] > 0) {
                values[cathode] =
                  (double)scalers[cathode][side][plane][RPC] / (double)elapsedTime[cathode][side][plane][RPC];
                //                printf("%d %d %d %llu %llu %f\n", plane, side, RPC,
                //                scalers[cathode][side][plane][RPC],
                //                       elapsedTime[cathode][side][plane][RPC], values[cathode]);
              }
            }
            runObjectBuffer[plane][side][RPC].setScalBending(values[0]);
            runObjectBuffer[plane][side][RPC].setScalNotBending(values[1]);
          }
        }
      }
    } else {
      printf("\t\tERROR: TriggerScalers not found for run %d\n", runIterator.first);
    }

    printf("\t\tINFO: Saving run %d\n", runIterator.first);

    for (int plane = MTRPlanes::kMT11; plane < MTRPlanes::kNPlanes; plane++) {
      for (int side = kINSIDE; side < MTRSides::kNSides; side++) {
        for (int RPC = k1; RPC < MTRRPCs::kNRPCs; RPC++) {
          fRunDataVect[plane][side][RPC].emplace_back(runObjectBuffer[plane][side][RPC]);
        }
      }
    }
  }

  for (int plane = MTRPlanes::kMT11; plane < MTRPlanes::kNPlanes; plane++) {
    for (int side = kINSIDE; side < MTRSides::kNSides; side++) {
      for (int RPC = k1; RPC < MTRRPCs::kNRPCs; RPC++) {
        std::sort(fRunDataVect[plane][side][RPC].begin(), fRunDataVect[plane][side][RPC].end(),
                  [](const RunObject& a, const RunObject& b) -> bool { return a.getRunNumber() < b.getRunNumber(); });
      }
    }
  }
}

void MTRShuttle::parseOCDBiMon(std::string path)
{

  AlienUtils::connectIfNeeded(path);

  AliCDBManager* managerCDB = AliCDBManager::Instance();

  int previousRunYear = 0;
  std::string CDBpath;

  for (const auto& runIterator : fRunList) {

    AliCDBStorage* defStorage = managerCDB->GetDefaultStorage();
    if (!defStorage)
      continue;

    managerCDB->SetRun(runIterator.first);
    defStorage->QueryCDB(runIterator.first);

    if (!AlienUtils::checkCDB(runIterator.first, defStorage, "MUON/Calib/TriggerDCS", false)) {
      printf("\t\tERROR: TriggerDCS not found for run %d\n", runIterator.first);
      continue;
    } else {
      printf("\t\tINFO: TriggerDCS found for run %d\n", runIterator.first);
    }

    // inizializzazione dell'entry contente i valori di corrente
    AliCDBEntry* entryDCS = managerCDB->Get("MUON/Calib/TriggerDCS");
    if (!entryDCS) {
      printf("\t\tERROR: AliCDBEntry not found for run %d\n", runIterator.first);
      continue;
    }

    // mappa delle correnti
    auto mapDCS = (TMap*)entryDCS->GetObject();
    if (!mapDCS) {
      printf("\t\tERROR: mapDCS not found for run %d\n", runIterator.first);
      continue;
    }

    // loop sui piani, i lati (inside e outside) e le RPC (9 per side)
    for (int plane = MTRPlanes::kMT11; plane < MTRPlanes::kNPlanes; plane++) {
      for (int side = kINSIDE; side < MTRSides::kNSides; side++) {
        for (int RPC = k1; RPC < MTRRPCs::kNRPCs; RPC++) {

          // creazione di un pointer all'elemento della mappa delle tensioni
          TObjArray* dataArrayVoltage;
          dataArrayVoltage = (TObjArray*)(mapDCS->GetValue(
            Form("MTR_%s_MT%d_RPC%d_HV.actual.iMon", kSides[side].c_str(), kPlanes[plane], RPC + 1)));

          if (!dataArrayVoltage) {
            printf(" Problems getting dataArrayCurrent\n");
            break;
          }

          // loop sulle entry del vettore di misure di tensione
          for (int arrayIndex = 0; arrayIndex < (dataArrayVoltage->GetEntries()); arrayIndex++) {
            auto* value = (AliDCSValue*)dataArrayVoltage->At(arrayIndex);

            value->GetTimeStamp();
            fAMANDACurrentsVect[plane][side][RPC].emplace_back(
              AMANDACurrent((uint64_t)value->GetTimeStamp(), value->GetFloat(), 0., false));
            std::cout << Form("MTR_%s_MT%d_RPC%d ", kSides[side].c_str(), kPlanes[plane], RPC + 1)
                      << fAMANDACurrentsVect[plane][side][RPC].back() << std::endl;
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
    mts[11] = 0;
    mts[12] = 1;
    mts[21] = 2;
    mts[22] = 3;

    std::string line;
    std::ifstream fin(path);
    AMANDACurrent bufferCurrent;
    if (fin.is_open()) {
      while (!fin.eof()) {
        getline(fin, line);
        if (line.empty())
          continue;
        std::cout << linesCounter++ << "\r";
        const char* charbuffer = (char*)line.c_str();
        if (!charbuffer)
          continue;
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
    } else
      std::cout << "Unable to open file";
  }

  for (int plane = MTRPlanes::kMT11; plane < MTRPlanes::kNPlanes; plane++) {
    for (int side = kINSIDE; side < MTRSides::kNSides; side++) {
      for (int RPC = k1; RPC < MTRRPCs::kNRPCs; RPC++) {
        std::sort(
          fAMANDACurrentsVect[plane][side][RPC].begin(), fAMANDACurrentsVect[plane][side][RPC].end(),
          [](const AMANDACurrent& a, const AMANDACurrent& b) -> bool { return a.getTimeStamp() < b.getTimeStamp(); });
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
    mts[11] = 0;
    mts[12] = 1;
    mts[21] = 2;
    mts[22] = 3;

    std::string line;
    std::ifstream fin(path);
    AMANDAVoltage bufferVoltage;
    if (fin.is_open()) {
      while (!fin.eof()) {
        getline(fin, line);
        if (line.empty())
          continue;
        std::cout << linesCounter++ << "\r";
        const char* charbuffer = (char*)line.c_str();
        if (!charbuffer)
          continue;
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
    } else
      std::cout << "Unable to open file";
  }

  for (int plane = MTRPlanes::kMT11; plane < MTRPlanes::kNPlanes; plane++) {
    for (int side = kINSIDE; side < MTRSides::kNSides; side++) {
      for (int RPC = k1; RPC < MTRRPCs::kNRPCs; RPC++) {
        std::sort(
          fAMANDAVoltagesVect[plane][side][RPC].begin(), fAMANDAVoltagesVect[plane][side][RPC].end(),
          [](const AMANDAVoltage& a, const AMANDAVoltage& b) -> bool { return a.getTimeStamp() < b.getTimeStamp(); });
      }
    }
  }

  std::cout << "Loaded " << linesCounter << "AMANDA voltages values" << std::endl;
}

void MTRShuttle::createDummyRuns(bool createFirstLast)
{

  std::cout << "Creating dummy runs...\n";

  uint64_t firstAMANDATSGlobal = std::numeric_limits<uint64_t>::max();
  uint64_t lastAMANDATSGlobal = 0;

  for (int plane = MTRPlanes::kMT11; plane < MTRPlanes::kNPlanes; plane++) {
    for (int side = kINSIDE; side < MTRSides::kNSides; side++) {
      for (int RPC = k1; RPC < MTRRPCs::kNRPCs; RPC++) {
        // Getting first AMANDA TS from iMon and vMon vectors
        auto firstAMANDAiMonTS = (!(fAMANDACurrentsVect[plane][side][RPC].empty()))
                                   ? fAMANDACurrentsVect[plane][side][RPC].begin()->fTimeStamp
                                   : std::numeric_limits<uint64_t>::max();
        auto firstAMANDAvMonTS = (!(fAMANDAVoltagesVect[plane][side][RPC].empty()))
                                   ? fAMANDAVoltagesVect[plane][side][RPC].begin()->fTimeStamp
                                   : std::numeric_limits<uint64_t>::max();

        // Retrieving the lowest TS
        auto firstAMANDATS = (firstAMANDAiMonTS < firstAMANDAvMonTS) ? firstAMANDAiMonTS : firstAMANDAvMonTS;

        if (firstAMANDATS < firstAMANDATSGlobal)
          firstAMANDATSGlobal = firstAMANDATS;

        // Getting last AMANDA TS from iMon and vMon vectors
        auto lastAMANDAiMonTS = (!(fAMANDACurrentsVect[plane][side][RPC].empty()))
                                  ? fAMANDACurrentsVect[plane][side][RPC].back().fTimeStamp
                                  : 0;
        auto lastAMANDAvMonTS = (!(fAMANDAVoltagesVect[plane][side][RPC].empty()))
                                  ? fAMANDAVoltagesVect[plane][side][RPC].back().fTimeStamp
                                  : 0;

        // Retrieving the highest TS
        auto lastAMANDATS = (lastAMANDAiMonTS < lastAMANDAvMonTS) ? lastAMANDAvMonTS : lastAMANDAiMonTS;

        if (lastAMANDATS > lastAMANDATSGlobal)
          lastAMANDATSGlobal = lastAMANDATS;
      }
    }
  }

  printf("Global TS limits {%llu,%llu}\n", firstAMANDATSGlobal, lastAMANDATSGlobal);

  for (int plane = MTRPlanes::kMT11; plane < MTRPlanes::kNPlanes; plane++) {
    for (int side = kINSIDE; side < MTRSides::kNSides; side++) {
      for (int RPC = k1; RPC < MTRRPCs::kNRPCs; RPC++) {
        // Sorting the vector of RunData
        std::sort(fRunDataVect[plane][side][RPC].begin(), fRunDataVect[plane][side][RPC].end(),
                  [](const RunObject& a, const RunObject& b) -> bool { return a.getSOR() < b.getSOR(); });
      }
    }
  }

  // First run of the RunData vector
  auto runObjectIt = fRunDataVect[MTRPlanes::kMT11][kINSIDE][k1].begin();
  auto firstRunTS = fRunDataVect[MTRPlanes::kMT11][kINSIDE][k1].begin()->getSOR();
  // Original last run of RunData vector
  auto runObjectEnd = fRunDataVect[MTRPlanes::kMT11][kINSIDE][k1].end();
  auto lastRunTS = fRunDataVect[MTRPlanes::kMT11][kINSIDE][k1].back().getEOR();

  printf("Run TS limits {%llu,%llu}\n", firstRunTS, lastRunTS);

  uint64_t runNumber = 1;

  for (; runObjectIt < runObjectEnd - 1; runObjectIt++) {
    auto dummySOR = runObjectIt->getEOR() + 1;
    auto dummyEOR = (runObjectIt + 1)->getSOR() - 1;

    std::cout << runObjectIt->getRunNumber() << std::endl;
    printf("Creating runs based on run %llu {%llu,%llu}.\n", runObjectIt->getRunNumber(), runObjectIt->getEOR() + 1,
           (runObjectIt + 1)->getSOR());

    for (int plane = MTRPlanes::kMT11; plane < MTRPlanes::kNPlanes; plane++) {
      for (int side = kINSIDE; side < MTRSides::kNSides; side++) {
        for (int RPC = k1; RPC < MTRRPCs::kNRPCs; RPC++) {
          if (dummySOR < dummyEOR) {
            fRunDataVect[plane][side][RPC].emplace_back(dummySOR, dummyEOR);
            fRunDataVect[plane][side][RPC].back().setRunNumber(runNumber);
            fRunDataVect[plane][side][RPC].back().setfIsDummy(true);
            printf("Created run %llu from %llu to %llu.\n", runNumber - 1, dummySOR, dummyEOR);
          }
        }
      }
    }
    runNumber++;
  }

  if (createFirstLast) {
    if (firstAMANDATSGlobal < firstRunTS) {
      for (int plane = MTRPlanes::kMT11; plane < MTRPlanes::kNPlanes; plane++) {
        for (int side = kINSIDE; side < MTRSides::kNSides; side++) {
          for (int RPC = k1; RPC < MTRRPCs::kNRPCs; RPC++) {
            fRunDataVect[plane][side][RPC].emplace_back(RunObject(firstAMANDATSGlobal, firstRunTS));
            fRunDataVect[plane][side][RPC].back().setRunNumber(0);
            fRunDataVect[plane][side][RPC].back().setfIsDummy(true);
            printf(
              //            "####################################\n"
              "Created first run %llu from %llu to %llu.\n", 0, fRunDataVect[plane][side][RPC].back().getSOR(),
              fRunDataVect[plane][side][RPC].back().getEOR());
          }
        }
      }
    }

    if (lastRunTS < lastAMANDATSGlobal) {
      for (int plane = MTRPlanes::kMT11; plane < MTRPlanes::kNPlanes; plane++) {
        for (int side = kINSIDE; side < MTRSides::kNSides; side++) {
          for (int RPC = k1; RPC < MTRRPCs::kNRPCs; RPC++) {
            fRunDataVect[plane][side][RPC].emplace_back(RunObject(lastRunTS + 1, lastAMANDATSGlobal));
            fRunDataVect[plane][side][RPC].back().setRunNumber(runNumber);
            fRunDataVect[plane][side][RPC].back().setfIsDummy(true);
            printf("Created last run %llu from %llu to %llu.\n"
                   //            "####################################\n"
                   ,
                   runNumber, fRunDataVect[plane][side][RPC].back().getSOR(),
                   fRunDataVect[plane][side][RPC].back().getEOR());
          }
        }
      }
    }
  }

  for (int plane = MTRPlanes::kMT11; plane < MTRPlanes::kNPlanes; plane++) {
    for (int side = kINSIDE; side < MTRSides::kNSides; side++) {
      for (int RPC = k1; RPC < MTRRPCs::kNRPCs; RPC++) {
        // Sorting the vector of RunData
        std::sort(fRunDataVect[plane][side][RPC].begin(), fRunDataVect[plane][side][RPC].end(),
                  [](const RunObject& a, const RunObject& b) -> bool { return a.getSOR() < b.getSOR(); });
      }
    }
  }
}

void MTRShuttle::setAMANDAIsHVOk(int plane, int side, int RPC)
{
  printf("MT%d %s RPC%d...\n Setting isHvOk... \n", kPlanes[plane], kSides[side].c_str(), RPC);

  // Creating a vector of validity intervals for HV
  std::vector<validityInterval> isHvOkIntervals;
  uint64_t firstTS = 0ull;
  uint64_t lastTS = 0ull;

  double HVThreshold = 8000.; // TODO: this value should not be hardcoded!

  // Looping over voltages readings
  for (auto voltageIt = fAMANDAVoltagesVect[plane][side][RPC].begin();
       voltageIt < fAMANDAVoltagesVect[plane][side][RPC].end() - 1; voltageIt++) {
    // When a good HV is found set firstTS to the actual TS
    if (voltageIt->getHV() > HVThreshold) {
      firstTS = voltageIt->getTimeStamp();
      // Then loop incrementing the iterator
      while (true) {
        voltageIt++;
        // When a low HV (or the back) is found get that TS as the lastTS and break
        if ((voltageIt->getHV() < HVThreshold) || (voltageIt + 1 == fAMANDAVoltagesVect[plane][side][RPC].end())) {
          lastTS = voltageIt->getTimeStamp();
          break;
        }
      }
      // Create a validity interval between firstTS and lastTS
      isHvOkIntervals.emplace_back(validityInterval(firstTS, lastTS));
      //      printf("HVOk interval {%llu,%llu}\n",firstTS,lastTS);
    }
  }

  // The instance of the iterator is external to the loop to avoid always starting from the first element
  auto currentIt = fAMANDACurrentsVect[plane][side][RPC].begin();

  // Loop over the defined validity intervals
  for (const auto& intervalIt : isHvOkIntervals) {
    // Loop over current readings (they are sorted!)
    for (; currentIt < fAMANDACurrentsVect[plane][side][RPC].end(); currentIt++) {
      // Skip values before intervals, break if the actual interval is passed, set isHvOk between the interval
      if (currentIt->getTimeStamp() < intervalIt.start)
        continue;
      else if (currentIt->getTimeStamp() > intervalIt.stop) {
        currentIt--;
        break;
      } else {
        currentIt->setIsHvOk(true);
      }
    }
  }
}

void MTRShuttle::setAMANDAIsDark(int plane, int side, int RPC)
{
  printf("Setting isDark... \n");

  std::sort(fAMANDACurrentsVect[plane][side][RPC].begin(), fAMANDACurrentsVect[plane][side][RPC].end(),
            [](const AMANDACurrent& a, const AMANDACurrent& b) -> bool { return a.getTimeStamp() < b.getTimeStamp(); });

  // Creating a vector of validity intervals for HV
  std::vector<validityInterval> isDarkIntervals;

  // The intervals corresponding to dark runs are delimited by SOR and EOR
  for (const auto& runObjectIt : fRunDataVect[plane][side][RPC]) {
    if (runObjectIt.isDark()) {
      isDarkIntervals.emplace_back(validityInterval(runObjectIt.getSOR(), runObjectIt.getEOR()));
      //      printf("Dark interval {%llu,%llu} (run %llu)\n",runObjectIt.getSOR(),
      //      runObjectIt.getEOR(),runObjectIt.getRunNumber());
    }
  }

  // The instance of the iterator is external to the loop to avoid always starting from the first element
  auto currentIt = fAMANDACurrentsVect[plane][side][RPC].begin();

  // Loop over the defined validity intervals
  for (const auto& intervalIt : isDarkIntervals) {
    // Loop over current readings (they are sorted!)
    for (; currentIt < fAMANDACurrentsVect[plane][side][RPC].end(); currentIt++) {
      // Skip values before intervals, break if the actual interval is passed, set isDark between the interval
      if (currentIt->getTimeStamp() < intervalIt.start)
        continue;
      else if (currentIt->getTimeStamp() > intervalIt.stop) {
        currentIt--;
        break;
      } else {
        if (currentIt->getITot() != 0.)
          currentIt->setIsDark(true);
      }
    }
  }
}

void MTRShuttle::setAMANDAiDark(int plane, int side, int RPC)
{
  printf("Setting iDark... \n");

  std::sort(fAMANDACurrentsVect[plane][side][RPC].begin(), fAMANDACurrentsVect[plane][side][RPC].end(),
            [](const AMANDACurrent& a, const AMANDACurrent& b) -> bool { return a.getTimeStamp() < b.getTimeStamp(); });

  // Iterator to point to the last dark current reading
  auto lastDarkIt = fAMANDACurrentsVect[plane][side][RPC].begin();

  while (!(lastDarkIt->isDark())) {
    lastDarkIt++;
  }

  bool wasPrevDark = lastDarkIt->isDark();

  // Loop over the current readings
  for (auto darkCurrentIt = fAMANDACurrentsVect[plane][side][RPC].begin() + 1;
       darkCurrentIt < fAMANDACurrentsVect[plane][side][RPC].end(); darkCurrentIt++) {

    if (darkCurrentIt->isDark()) {
      // If previous reading and current one are dark, update last dark
      if (wasPrevDark && !(darkCurrentIt->getIDark() == 0.)) {
        lastDarkIt = darkCurrentIt;
        // If previous reading wasn't dark, while current one is, set dark currents from lastDark to darkCurrentIt
      } else if (!wasPrevDark) {
        // Computing the parameters for the "dumb interpolation"
        double m = getM(*lastDarkIt, *darkCurrentIt);
        double q = getQ(*lastDarkIt, *darkCurrentIt);
        double TS0 = lastDarkIt->getTimeStamp();

        // Assigning dark current values from interpolation to the not-dark readings
        if (lastDarkIt + 1 < darkCurrentIt - 1) {
          std::for_each(lastDarkIt + 1, darkCurrentIt, [&m,&q,&TS0](AMANDACurrent& reading) {
            reading.setIDark(m * (reading.getTimeStamp() - TS0) + q);
          });
        }
      }
    }

    // Keep track of the "darkness" of the actual current reading
    wasPrevDark = darkCurrentIt->isDark();
    if(wasPrevDark) lastDarkIt = darkCurrentIt;
  }
}

void MTRShuttle::propagateAMANDAVoltage(int plane, int side, int RPC, bool weightedAverage)
{
  printf("Setting voltage... \n");

  auto voltageIt = fAMANDAVoltagesVect[plane][side][RPC].begin();

  for (auto& runObjectIt : fRunDataVect[plane][side][RPC]) {

    // Load SOR and EOR values
    auto SOR = runObjectIt.getSOR();
    auto EOR = runObjectIt.getEOR();

    double hvCumulus = 0.;
    double totalT = 0.;
    int iCounter = 0;

    // Loop over the voltage readings
    for (; voltageIt != fAMANDAVoltagesVect[plane][side][RPC].end() - 1; voltageIt++) {

      auto TS = voltageIt->getTimeStamp();

      // If the timestamp is after the EOR break the loop (aka pass to the following run)
      if (TS > EOR) {
        if(voltageIt!=fAMANDAVoltagesVect[plane][side][RPC].begin()) voltageIt--;
        break;
      }

      auto nextTS = (voltageIt + 1)->getTimeStamp();

      // If the timestamp is before the SOR
      if (TS < SOR) {
        // If nextTS is before SOR skip
        if (nextTS < SOR)
          continue;
        // Else the current value has to be averaged from SOR to nextTS
        else
          TS = SOR;
      }

      // If nextTS is after EOR the current value has to be averaged from TS to EOR
      if (nextTS >= EOR)
        nextTS = EOR;

      // Compute deltaT
      auto deltaT = nextTS - TS;

//      std::cout << voltageIt->getHV() <<","<< deltaT <<","<< hvCumulus <<std::endl;

      // Add voltage value to average numerator sum
      if (weightedAverage) {
        hvCumulus += voltageIt->getHV() * (double)deltaT;
      } else {
        hvCumulus += voltageIt->getHV();
      }

      // Now deltaT should always be equal to EOR-SOR
      totalT += deltaT;
      iCounter++;
    }

    // Compute average and assign values to run object
    if (weightedAverage) {
      runObjectIt.setAvgHV((totalT > 0) ? hvCumulus / totalT : runObjectIt.getAvgHV());
    } else {
      runObjectIt.setAvgHV((iCounter > 0) ? hvCumulus / (double)iCounter : runObjectIt.getAvgHV());
    }

    if (runObjectIt.getAvgHV() > 8000.)
      runObjectIt.setfIsHVOk(true);

//    std::cout << "run " << runObjectIt.getRunNumber() << " had " << iCounter << " available voltage readings giving "
//              << runObjectIt.getAvgHV() << ".\n";
  }
}

void MTRShuttle::propagateAMANDACurrent(int plane, int side, int RPC, bool weightedAverage)
{
  auto currentIt = fAMANDACurrentsVect[plane][side][RPC].begin();

  printf("Integrating and averaging... \n");

  for (auto& runObjectIt : fRunDataVect[plane][side][RPC]) {

    // Load SOR and EOR values
    auto SOR = runObjectIt.getSOR();
    auto EOR = runObjectIt.getEOR();

//    if(plane==kMT22 && side==kINSIDE && RPC==k3 )std::cout<<runObjectIt<<" {"<<SOR<<","<<EOR<<"}\n";

    double iDarkCumulus = 0.;
    double iTotCumulus = 0.;
    double totalT = 0.;
    int iCounter = 0;

    double integratedCharge = 0;

    // Loop over the current readings
    for (; currentIt != fAMANDACurrentsVect[plane][side][RPC].end() - 1; currentIt++) {

      auto TS = currentIt->getTimeStamp();

      // If the timestamp is after the EOR break the loop (aka pass to the following run)
      if (TS > EOR) {
        if(currentIt!=fAMANDACurrentsVect[plane][side][RPC].begin()) currentIt--;
        break;
      }

      auto nextTS = (currentIt + 1)->getTimeStamp();

      // If the timestamp is before the SOR
      if (TS < SOR) {
        // If nextTS is before SOR skip
        if (nextTS < SOR)
          continue;
        // Else the current value has to be averaged from SOR to nextTS
        else
          TS = SOR;
      }

      // If nextTS is after EOR the current value has to be averaged from TS to EOR
      if (nextTS >= EOR)
        nextTS = EOR;

      // Compute deltaT
      auto deltaT = nextTS - TS;
      if(currentIt->getINet()==0.) deltaT=0.;

//      std::cout << currentIt->getINet() <<","<< deltaT <<","<< integratedCharge <<std::endl;

      // Integrate current is HV is at working point
      if (currentIt->isHvOk() || !(currentIt->hasBeenFlagged())) {
        integratedCharge += currentIt->getINet() * (double) deltaT;
//        if(plane==kMT22 && side==kINSIDE && RPC==k3 ) std::cout<<currentIt->getTimeStamp()<<","<<currentIt->getINet()<<","<<deltaT<<","<<integratedCharge<<std::endl;
      }

      // Add current value to average numerator sum
      if (weightedAverage) {
        iDarkCumulus += currentIt->getIDark() * (double)deltaT;
        iTotCumulus += currentIt->getITot() * (double)deltaT;
      } else {
        iDarkCumulus += currentIt->getIDark();
        iTotCumulus += currentIt->getITot();
      }

      // Now deltaT should always be equal to EOR-SOR
      totalT += deltaT;
      iCounter++;
    }

    // Compute average and assign values to run object
    if (weightedAverage) {
      runObjectIt.setAvgIDark((totalT > 0) ? iDarkCumulus / totalT : 0.);
      runObjectIt.setAvgITot((totalT > 0) ? iTotCumulus / totalT : 0.);
      runObjectIt.setIntCharge(integratedCharge);
    } else {
      runObjectIt.setAvgIDark((iCounter > 0) ? iDarkCumulus / (double)iCounter : 0.);
      runObjectIt.setAvgITot((iCounter > 0) ? iTotCumulus / (double)iCounter : 0.);
      runObjectIt.setIntCharge(integratedCharge);
    }

//    if(plane==kMT22 && side==kINSIDE && RPC==k3 ) std::cout<<"############## "<<runObjectIt<<"\n";

//    std::cout << "run " << runObjectIt.getRunNumber() << " had " << iCounter << " available current readings giving "
//              << runObjectIt.getAvgINet() << runObjectIt.getIntCharge() << ".\n";
  }
}

void MTRShuttle::propagateAMANDA(bool weightedAverage)
{
  for (int plane = MTRPlanes::kMT11; plane < MTRPlanes::kNPlanes; plane++) {
    for (int side = kINSIDE; side < MTRSides::kNSides; side++) {
      for (int RPC = k1; RPC < MTRRPCs::kNRPCs; RPC++) {
        setAMANDAIsHVOk(plane, side, RPC);
        setAMANDAIsDark(plane, side, RPC);
        setAMANDAiDark(plane, side, RPC);

        propagateAMANDAVoltage(plane, side, RPC, weightedAverage);
        propagateAMANDACurrent(plane, side, RPC, weightedAverage);
      }
    }
  }
}

void MTRShuttle::computeAverage()
{
  // All data vectors should have the same runs at the moment. MT11IN1 is a generic choice.
  auto nOfRuns = fRunDataVect[MTRPlanes::kMT11][MTRSides::kINSIDE][MTRRPCs::k1].size();

  RunObject runDataBuffer;

  // Looping over runs
  for (int iRun = 0; iRun < (int)nOfRuns; iRun++) {
    // The run number is necessary to look for runs in the next RPCs
    auto currentRunNumber = fRunDataVect[MTRPlanes::kMT11][MTRSides::kINSIDE][MTRRPCs::k1][iRun].getRunNumber();

    // Loading the constant values
    runDataBuffer.setSOR(fRunDataVect[MTRPlanes::kMT11][MTRSides::kINSIDE][MTRRPCs::k1][iRun].getSOR());
    runDataBuffer.setEOR(fRunDataVect[MTRPlanes::kMT11][MTRSides::kINSIDE][MTRRPCs::k1][iRun].getEOR());
    runDataBuffer.setRunNumber(fRunDataVect[MTRPlanes::kMT11][MTRSides::kINSIDE][MTRRPCs::k1][iRun].getRunNumber());
    runDataBuffer.setfIsDark(fRunDataVect[MTRPlanes::kMT11][MTRSides::kINSIDE][MTRRPCs::k1][iRun].isDark());

    //        printf("Current runNumber=%llu\n",currentRunNumber);

    // Looping over all the RPCs
    for (int plane = MTRPlanes::kMT11; plane < MTRPlanes::kNPlanes; plane++) {

      auto nOfRPC = 0.;
      fRunDataVectAvg[plane].reserve(nOfRuns);

      for (int side = kINSIDE; side < MTRSides::kNSides; side++) {
        for (int RPC = k1; RPC < MTRRPCs::kNRPCs; RPC++) {

          // Try to find information regarding current run
          auto currentRun = std::find_if(
            fRunDataVect[plane][side][RPC].begin(), fRunDataVect[plane][side][RPC].end(),
            [currentRunNumber](const RunObject& run) -> bool { return run.getRunNumber() == currentRunNumber; });

          // If not found skip
          if (currentRun == fRunDataVect[plane][side][RPC].end())
            continue;

          // If the HV is ok take the run into account
          if ((currentRun->getAvgHV() > kMinWorkHV)) {
            runDataBuffer = runDataBuffer + *currentRun;
            nOfRPC++;
            //                                    std::cout << (*currentRun).getAvgITot() << "\n";
          }
        }
      }

      if (nOfRPC != 0)
        runDataBuffer = runDataBuffer / (double)nOfRPC;

      //                  printf("\nAverage current=%f with %f readings\n",runDataBuffer.getAvgITot(),nOfRPC);

      if (runDataBuffer.getAvgHV() > 8000.)
        runDataBuffer.setfIsHVOk(true);

      fRunDataVectAvg[plane].emplace_back(runDataBuffer);
      runDataBuffer.reset();
    }

    RunObject runDataTot;
    runDataTot.setSOR(fRunDataVect[MTRPlanes::kMT11][MTRSides::kINSIDE][MTRRPCs::k1][iRun].getSOR());
    runDataTot.setEOR(fRunDataVect[MTRPlanes::kMT11][MTRSides::kINSIDE][MTRRPCs::k1][iRun].getEOR());
    runDataTot.setRunNumber(fRunDataVect[MTRPlanes::kMT11][MTRSides::kINSIDE][MTRRPCs::k1][iRun].getRunNumber());
    runDataTot.setfIsDark(fRunDataVect[MTRPlanes::kMT11][MTRSides::kINSIDE][MTRRPCs::k1][iRun].isDark());

    fRunDataVectAvg[4].reserve(nOfRuns);

    for (int plane = 0; plane < kNPlanes; plane++) {
      auto currentRun = std::find_if(
        fRunDataVectAvg[plane].begin(), fRunDataVectAvg[plane].end(),
        [currentRunNumber](const RunObject& run) -> bool { return run.getRunNumber() == currentRunNumber; });
      runDataTot = runDataTot + *currentRun;
    }

    runDataTot = runDataTot / (double)kNPlanes;

    if (runDataTot.getAvgHV() > 8000.)
      runDataTot.setfIsHVOk(true);

    fRunDataVectAvg[4].emplace_back(runDataTot);
    runDataTot.reset();
  }
}

void MTRShuttle::saveData(std::string path)
{
  std::ofstream outputFile(path.c_str());

  for (int plane = MTRPlanes::kMT11; plane < MTRPlanes::kNPlanes; plane++) {
    for (int side = kINSIDE; side < MTRSides::kNSides; side++) {
      for (int RPC = k1; RPC < MTRRPCs::kNRPCs; RPC++) {
        for (const auto& dataIt : fRunDataVect[plane][side][RPC]) {
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
  std::ifstream fin(path);
  int linesCounter = 0;
  int plane, side, RPC;
  RunObject runObjectBuffer;

  if (fin.is_open()) {
    while (!fin.eof()) {
      getline(fin, line);
      if (line.empty())
        continue;
      std::cout << "Loaded lines: " << linesCounter++ << "\r";
      runObjectBuffer = RunObject(line, plane, side, RPC);
      fRunDataVect[plane][side][RPC].emplace_back(runObjectBuffer);
    }
    std::cout << std::endl;
    fin.close();
  } else
    std::cout << "Unable to open file";
}
