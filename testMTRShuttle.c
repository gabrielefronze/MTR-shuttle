//
// Created by Gabriele Gaetano Fronzé on 05/02/2018.
//

#include "MTRShuttle.h"

void testMTRShuttle(){
  MTRShuttle sciattol;

  sciattol.parseRunList("../runs_2010_2017_ALL");
  sciattol.parseOCDB("local:///Users/Gabriele/cernbox/Dottorato/MTR2017/CDB");
  sciattol.parseAMANDAiMon("../IMon2010_2017_UTF8.txt");
  sciattol.propagateAMANDA();
  for( const auto &it : sciattol.fRunDataVectAvg[4]){
    std::cout << it.isDark();
  }
  std::cout << std::endl;
  sciattol.saveData("MTR2010_2017_ALL_test.csv");
}