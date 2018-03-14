//
// Created by Gabriele Gaetano Fronzé on 01/02/2018.
//

#ifndef MTR_SHUTTLE_ALIENUTILS_H
#define MTR_SHUTTLE_ALIENUTILS_H

#include <stdio.h>
#include <stdlib.h>
#include <string>

namespace AlienUtils
{
    const int kRunYears[][3] = {{2010,105524,139698},
                                {2011,139699,170718},
                                {2012,170719,194308},
                                {2013,194309,199177},
                                {2014,200001,208364},
                                {2015,208365,247170},
                                {2016,247171,267254},
                                {2017,267255,282900}};

    static bool checkAlienToken() {
      std::string checkTokenBashCommand = R"(if [[ "`alien-token-info | grep "still valid" `" != "" ]]; then echo "1"; else echo "0"; fi)";
      bool returnValue = false;
      FILE * f = popen(checkTokenBashCommand.c_str(), "r");
      char buf[1];
      size_t read = fread((void *)&buf[0], 1, 1, f);
      returnValue = (buf[0] == '1');
      pclose(f);
      return returnValue;
    }

    static void initAlienToken(std::string userName){
      std::string command = "alien-token-init "+userName;
      system(command.c_str());
    }

    static bool connectIfNeeded(std::string path){
      if (path.find("alien") != std::string::npos) {
        if (!AlienUtils::checkAlienToken()) {
          std::string userName;
          std::cerr << "Alien token is not valid" << std::endl;
          std::cout << "Please enter your Alien username " << std::endl;
          std::cin >> userName;
          AlienUtils::initAlienToken(userName);
          if (!AlienUtils::checkAlienToken()) {
            std::cerr << "Alien token not valid even after reset. Proceed manually!" << std::endl;
            return false;
          }
        }
      }
      return true;
    }

    static int getRunYear(int runNumber){
      for (int iYear = 0; iYear < 8; ++iYear) {
        if ( runNumber<=kRunYears[iYear][2] && runNumber>=kRunYears[iYear][1]) return kRunYears[iYear][0];
      }
      return 0;
    }

    static bool checkCDB(int runNumber, AliCDBStorage *defStorage, TString path, bool defaultAllowed){
      TObjArray *arrCDBID = defStorage->GetQueryCDBList();
      if (!arrCDBID) return false;

      TIter nxt(arrCDBID);
      AliCDBId *cdbID = 0;

      while ((cdbID = (AliCDBId *) nxt())) {
        if (cdbID->GetPath() == path) {

//          printf("Validity %d %d\n",cdbID->GetFirstRun(),cdbID->GetLastRun());

          if (cdbID->GetFirstRun()==0) continue;

          if (cdbID->GetFirstRun()==runNumber){
            return true;
          }

          if (cdbID->GetFirstRun()==0 && defaultAllowed){
            printf("\t\tINFO: Only default file found.\n");
            return defaultAllowed;
          }
        }
      }
      return false;
    }
};

#endif //MTR_SHUTTLE_ALIENUTILS_H
