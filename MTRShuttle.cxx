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
#include <AliMpCDB.h>
#include <AliDCSValue.h>
#include <TClonesArray.h>
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
    fRunList.push_back(runBuffer);
  }
  fin.close();
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

    int RunYear = 2017;

    //i manager puntano al run desiderato
    managerCDB->SetRun(runIterator);

    AliCDBStorage *defStorage = managerCDB->GetDefaultStorage();
    if (!defStorage) continue;

    defStorage->QueryCDB(runIterator);
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
      //printf("\n\nSkipping run %d\n\n",runIterator);
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
            Form("MTR_%s_MT%d_RPC%d_HV.vEff", kSides[side], kPlanes[plane], RPC + 1)));

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
          runObjectBuffer[plane][side][RPC].setRunNumber((uint64_t) runIterator);
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
        }
      }
    }

    printf("scalers reading complete.\n");
  }


}

void MTRShuttle::parseAMANDAiMon(std::string path)
{
  int mts[23];
  mts[11]=0;
  mts[12]=1;
  mts[21]=2;
  mts[22]=3;

  bool isZero=false;

  uint64_t dummyTimeStamp=0;
  double timeStamp=0;
  double current=0.;
  int MT=0;
  int RPC=0;
  char InsideOutside='I';

  std::string line;
  std::ifstream fin (path);
  int linesCounter = 0;
  AMANDACurrent bufferCurrent(0., 0., false);
  if (fin.is_open())
  {
    while (! fin.eof() )
    {
      getline (fin,line);
      if (fin.eof()) break;
      std::cout<<linesCounter++<<"\r";
      const char *charbuffer = (char*)line.c_str();
      if (!charbuffer) continue;
      sscanf(charbuffer,"%llu;MTR_%c",&dummyTimeStamp,&InsideOutside);
      char pattern[200];
      sprintf(pattern,"%%lf;MTR_%sSIDE_MT%%d_RPC%%d_HV.actual.iMon;%%lf",(InsideOutside=='I'?"IN":"OUT"));
      sscanf(charbuffer,pattern,&timeStamp,&MT,&RPC,&current);
      bufferCurrent.setITot(current);
      fAMANDACurrentsVect[mts[MT]][(InsideOutside=='I'?0:1)][RPC-1].push_back(bufferCurrent);
    }
    std::cout<<std::endl;
    fin.close();
  }
  else std::cout << "Unable to open file";

  for (int plane=0; plane<kNPlanes; plane++) {
    for (int side = 0; side < kNSides; side++) {
      for (int RPC = 0; RPC < kNRPC; RPC++) {
        std::sort(fAMANDACurrentsVect[plane][side][RPC].begin(),
                  fAMANDACurrentsVect[plane][side][RPC].end(),
                  [](const AMANDACurrent &a, const AMANDACurrent &b) -> bool {
                    return a.getTimeStamp() > b.getTimeStamp();
                  });
      }
    }
  }

  std::cout << "Loaded " << linesCounter << "AMANDA values" << std::endl;
}

void MTRShuttle::setIDark()
{
  for (int plane=0; plane<kNPlanes; plane++) {
    for (int side = 0; side < kNSides; side++) {
      for (int RPC = 0; RPC < kNRPC; RPC++) {

        auto setIsDarkIt= fAMANDACurrentsVect[plane][side][RPC].begin();

        for (const auto &runObjectIt: fRunDataVect[plane][side][RPC]) {

          if (!runObjectIt.getfIsDark()) continue;

          auto SOR = runObjectIt.getSOR();
          auto EOR = runObjectIt.getEOR();

          for ( setIsDarkIt; setIsDarkIt!=fAMANDACurrentsVect[plane][side][RPC].end(); setIsDarkIt++) {

            auto TS = setIsDarkIt->getTimeStamp();

            if ( TS < SOR ) continue;
            else if ( TS > EOR ){
              setIsDarkIt--;
              break;
            } else {
              setIsDarkIt->setIDark(setIsDarkIt->getITot());
              setIsDarkIt->setIsDark(true);
            }
          }
        }

        // Iterator to point to the last dark current reading
        auto lastDarkIt = fAMANDACurrentsVect[plane][side][RPC].begin();
        bool wasPrevDark = lastDarkIt->isDark();

        // Loop over the current readings
        for (auto darkCurrentIt=fAMANDACurrentsVect[plane][side][RPC].begin();
             darkCurrentIt!=fAMANDACurrentsVect[plane][side][RPC].end();
             darkCurrentIt++) {

          // If previous reading and current one are dark update last dark
          if ( wasPrevDark && darkCurrentIt->isDark() ) lastDarkIt = darkCurrentIt;

          // If previous reading wasn't dark, while current one yes, set dark currents
          else if ( !wasPrevDark && darkCurrentIt->isDark()) {

            // Computing the parameters for the "dumb interpolation"
            double m = getM(*lastDarkIt,*darkCurrentIt);
            double q = getQ(*lastDarkIt,*darkCurrentIt);

            // Assigning dark current values from interpolation to the not-dark readings
            std::for_each(lastDarkIt+1,darkCurrentIt-1,[m,q](AMANDACurrent reading){
              reading.setIDark(m*reading.getTimeStamp()+q);
              return;
            });
          }

          wasPrevDark = darkCurrentIt->isDark();
        }
      }
    }
  }
}
