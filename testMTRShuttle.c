//
// Created by Gabriele Gaetano Fronzé on 05/02/2018.
//

#include <iostream>
#include "MTRShuttle.h"

void testMTRShuttle(){
  MTRShuttle sciattol;

  //  sciattol.instance("runListTest.txt","Imon1018.txt","vMon1018.txt","local:///Users/Gabriele/cernbox/Dottorato/MTR2017/CDB/####/OCDB");
//  sciattol.instance("../runs_2010_2017_ALL","Imon1018.txt","vMon1018.txt","local:///Users/Gabriele/cernbox/Dottorato/MTR2017/CDB/####/OCDB",true,true);
  sciattol.instance("../runs_2010_2017_ALL","Imon1018.txt","vMon1018.txt","local:///Users/Gabriele/cernbox/Dottorato/MTR2017/CDB/####/OCDB",true,false);
//  sciattol.instance("run196474.txt","Imon1018.txt","vMon1018.txt","local:///Users/Gabriele/cernbox/Dottorato/MTR2017/CDB/####/OCDB");

//  sciattol.parseRunList("../runlists_Max/allruns_2016-2017.txt");
//  sciattol.parseRunList("../runs_2017_test90");
//  sciattol.parseRunList("../runs_2010_2017_ALL");
//  sciattol.parseRunList("runsTestPropagation");
//  sciattol.parseRunList("run196474.txt");
//  sciattol.parseOCDB("local:///Users/Gabriele/cernbox/Dottorato/MTR2017/CDB/####/OCDB");
//  sciattol.parseAMANDAiMon("Imon1018.txt");
//  sciattol.parseAMANDAvMon("vMon1018.txt");
//  sciattol.propagateAMANDA(true);
//  sciattol.computeAverage();

  // for( const auto &it : sciattol.fRunDataVectAvg[4]){
  //   std::cout << it.isDark();
  // }
  // std::cout << std::endl;

  sciattol.saveData("testALL.csv");
}