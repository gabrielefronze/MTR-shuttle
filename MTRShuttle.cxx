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

  AliCDBManager *managerPrototype = AliCDBManager::Instance();

  for (const auto &runIterator : fRunList) {
    AliCDBManager *managerYearCheck = managerPrototype;

    int RunYear=2017;

    managerPrototype->SetDefaultStorage(path.c_str());

    //i manager puntano al run desiderato
    managerPrototype->SetRun(runIterator);

    if(!managerPrototype) continue;

    AliCDBManager *managerCurrent = managerPrototype;
    AliCDBManager *managerVoltage = managerPrototype;
    AliCDBManager *managerRunType = managerPrototype;
    AliCDBManager *managerScaler  = managerPrototype;

    AliCDBStorage *defStorage = managerCurrent->GetDefaultStorage();
    if(!defStorage) continue;

    defStorage->QueryCDB(runIterator);
    TObjArray* arrCDBID = defStorage->GetQueryCDBList();
    if(!arrCDBID) continue;
    TIter nxt(arrCDBID);
    AliCDBId* cdbID = 0;
    bool hasGRP = kFALSE;
    while ((cdbID=(AliCDBId*)nxt())) {
      if (cdbID->GetPath() == "GRP/GRP/Data") {hasGRP = kTRUE; break;}
    }
    if(!hasGRP){
      //printf("\n\nSkipping run %d\n\n",runIterator);
      continue;
    }

    if(!AliMpCDB::LoadDDLStore()) continue;
    AliMpDDLStore *ddlStore=AliMpDDLStore::Instance();

    //inizializzazione dell'entry contente il runtype
    AliCDBEntry *entryRunType = managerRunType->Get("GRP/GRP/Data");
    if(!entryRunType) continue;

    //retrievering delle informazioni sul run
    auto *grpObj=(AliGRPObject*)entryRunType->GetObject();
    if(!grpObj) continue;
    TString runType = grpObj->GetRunType();
    TString beamType = grpObj->GetBeamType();
    float beamEnergy = grpObj->GetBeamEnergy();
    TString LHCState = grpObj->GetLHCState();
    auto SOR=(uint64_t)grpObj->GetTimeStart();
    auto EOR=(uint64_t)grpObj->GetTimeEnd();

    bool isDark = false;
    //settaggio del flag beamPresence
    bool isBeamPresent = (beamEnergy > 1.);

    //settaggio del flag isDark
    if(runType.Contains("PHYSICS")){
      isDark=false;
      //cout<<(*runIterator).runNumber<<" is phys"<<endl;
    } else if(runType.Contains("CALIBRATION")){
      isDark=true;
      //cout<<(*runIterator).runNumber<<" is calib"<<endl;
    } else {
      continue;
    }

    //inizializzazione dell'entry contente i valori di corrente
    AliCDBEntry *entryDCS = managerCurrent->Get("MUON/Calib/TriggerDCS");
    if(!entryDCS) continue;

    //mappa delle correnti
    TMap *mapDCS = (TMap*)entryDCS->GetObject();
    if(!mapDCS) continue;

    //inizializzazone dell'entry contenente le letture degli scalers
    AliCDBEntry *entryScalers = managerScaler->Get("MUON/Calib/TriggerScalers");
    if(!entryScalers) continue;

    //array delle letture
    TClonesArray *arrayScalers = (TClonesArray*)entryScalers->GetObject();
    if(!arrayScalers) continue;

    RunObject runObjectBuffer[kNPlanes][kNSides][kNRPC];

    //loop sui piani, i lati (inside e outside) e le RPC (9 per side)
    for (int plane=0; plane<kNPlanes; plane++) {
      for (int side=0; side<kNSides; side++) {
        for (int RPC=0; RPC<kNRPC; RPC++) {

          //creazione di un pointer all'elemento della mappa delle tensioni
          TObjArray *dataArrayVoltage;
          dataArrayVoltage=(TObjArray*)(mapDCS->GetValue(Form("MTR_%s_MT%d_RPC%d_HV.vEff",kSides[side],kPlanes[plane],RPC+1)));

          if(!dataArrayVoltage) {
            printf(" Problems getting dataArrayVoltage\n");
            break;
          }

          bool isVoltageOk=true;
          double avgHV = 0.;
          int counterHV = 0;

          //loop sulle entry del vettore di misure di tensione
          for (int arrayIndex=0; arrayIndex<(dataArrayVoltage->GetEntries()); arrayIndex++) {
            auto *value = (AliDCSValue*)dataArrayVoltage->At(arrayIndex);

            auto HV = value->GetFloat();

            if(HV<8500.){
              isVoltageOk=kFALSE;
              break;
            } else {
              avgHV+=HV;
              counterHV++;
            }
          }

          runObjectBuffer[plane][side][RPC].setfIsDark(isDark && !(isBeamPresent));
          runObjectBuffer[plane][side][RPC].setRunNumber((uint64_t) runIterator);
          runObjectBuffer[plane][side][RPC].setSOR(SOR);
          runObjectBuffer[plane][side][RPC].setEOR(EOR);
          runObjectBuffer[plane][side][RPC].setAvgHV((counterHV!=0) ? avgHV/counterHV : 0.);

          if (!isVoltageOk) break;
        }
      }
    }

    uint64_t elapsedTime=0;
    UInt_t readLB[kNCathodes][kNSides][kNPlanes][kNRPC];
    UInt_t overflowLB[kNCathodes][kNSides][kNPlanes][kNRPC];
    for (int plane=0; plane<kNPlanes; plane++) {
      for (int side=0; side<kNSides; side++) {
        for (int RPC=1; RPC<=kNRPC; RPC++) {
          for (int cathode=0; cathode<kNCathodes; cathode++) {
            readLB[cathode][side][plane][RPC-1]=0;
            overflowLB[cathode][side][plane][RPC-1]=0;
          }
        }
      }
    }

    //loop sulle entries, sui piani, i catodi (bending e non bending) e le Local Boards (234 per piano)
    AliMUONTriggerScalers *scalersData;
    for (int scalerEntry=0; scalerEntry<arrayScalers->GetEntries(); scalerEntry++){
      scalersData=(AliMUONTriggerScalers*)arrayScalers->At(scalerEntry);
      Int_t arrayScalersEntries=arrayScalers->GetEntries();
      elapsedTime+=scalersData->GetDeltaT();

      for (int plane=0; plane<kNPlanes; plane++) {
        for (int cathode=0; cathode<kNCathodes; cathode++) {
          for (int localBoard=0; localBoard<kNLocalBoards; localBoard++) {
            Int_t iRPC017=(ddlStore->GetDEfromLocalBoard(localBoard+1, plane+10))-(plane+1+10)*100;
            Int_t iRPC09=kRPCIndexes[iRPC017];
            Int_t iSide=kRPCSides[iRPC017];

            // se c'è overflow scarto il dato
            readLB[cathode][iSide][plane][iRPC09-1]++;

            bool isOverflow=false;
            if (scalersData->GetLocScalStripOver(cathode, plane, localBoard)!=0. ){
              overflowLB[cathode][iSide][plane][iRPC09-1]++;
              readLB[cathode][iSide][plane][iRPC09-1]--;
              isOverflow=true;
            }

            // se la lettura non è quella a fine run immagazzino il dato con timestamp pari a SOR+DeltaT
            if(scalerEntry!=arrayScalersEntries-1){
              fOCDBRPCScalersTreeBufferW[cathode][iSide][plane][iRPC09-1] = new AliRPCValueScaler(runIterator, SOR+elapsedTime,RunYear, scalersData->GetLocScalStrip(cathode, plane, localBoard), isDark,*beamType,beamEnergy,*LHCState, scalersData->GetDeltaT(), isOverflow);
            }
              // altrimenti il timestamp è pari all'EOR
            else {
              fOCDBRPCScalersTreeBufferW[cathode][iSide][plane][iRPC09-1] = new AliRPCValueScaler(runIterator, EOR, RunYear,scalersData->GetLocScalStrip(cathode, plane, localBoard), isDark,*beamType,beamEnergy,*LHCState, scalersData->GetDeltaT(), isOverflow);
            }

            fOCDBRPCScalersTree[cathode][iSide][plane][iRPC09-1]->SetBranchAddress(fOCDBRPCScalersTree[cathode][iSide][plane][iRPC09-1]->GetTitle(),&fOCDBRPCScalersTreeBufferW[cathode][iSide][plane][iRPC09-1]);
            fOCDBLBScalersTree[cathode][plane][localBoard]->SetBranchAddress(fOCDBLBScalersTree[cathode][plane][localBoard]->GetTitle(),&fOCDBRPCScalersTreeBufferW[cathode][iSide][plane][iRPC09-1]);

            fOCDBRPCScalersTree[cathode][iSide][plane][iRPC09-1]->Fill();
            fOCDBLBScalersTree[cathode][plane][localBoard]->Fill();

            delete fOCDBRPCScalersTreeBufferW[cathode][iSide][plane][iRPC09-1];

//                        printf(" MTR %d cathode %d LB %d RPC %d or %d_%s timestamp %lu fAMANDAData %d\n",fPlanes[plane],cathode,localBoard,iRPC017,iRPC09,(fSides[iSide]).Data(),SOR+elapsedTime,scalersData->GetLocScalStrip(cathode, plane, localBoard));
          }
        }
      }
      scalersData=nullptr;
    }
    delete scalersData;

    printf("scalers reading complete.\n");
  }


}

void MTRShuttle::parseAMANDA(std::string path)
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
  AMANDACurrent bufferCurrent(0.,0.);
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

  std::cout << "Loaded " << linesCounter << "AMANDA values" << std::endl;
}
