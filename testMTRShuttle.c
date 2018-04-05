//
// Created by Gabriele Gaetano Fronzé on 05/02/2018.
//

#include <iostream>
#include "MTRShuttle.h"

void testMTRShuttle(){
  MTRShuttle sciattol;

//  sciattol.parseRunList("../runlists_Max/allruns_2016-2017.txt");
//  sciattol.parseRunList("../runs_2017_test90");
  sciattol.parseRunList("../crosscheck_Max/runlist_calib.txt");
//  sciattol.parseRunList("runsTestPropagation");
  sciattol.parseOCDB("local:///Users/Gabriele/cernbox/Dottorato/MTR2017/CDB/####/OCDB");
  sciattol.parseAMANDAiMon("../IMon2017_UTF8.txt");
  sciattol.propagateAMANDA();
  sciattol.computeAverage();

  for( const auto &it : sciattol.fRunDataVectAvg[4]){
    std::cout << it.isDark();
  }
  std::cout << std::endl;

  sciattol.saveData("../crosscheck_Max/test_calib.csv");
}