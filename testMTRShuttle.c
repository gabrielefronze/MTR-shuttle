//
// Created by Gabriele Gaetano Fronzé on 05/02/2018.
//

#include "MTRShuttle.h"

void testMTRShuttle(){
  MTRShuttle sciattol;

  sciattol.parseRunList("../runs_2017_last");
  sciattol.parseOCDB("local:///Users/Gabriele/cernbox/Dottorato/MTR2017/CDB");
  sciattol.parseAMANDAiMon("../IMon2017_UTF8.txt");
  sciattol.propagateAMANDA();
  sciattol.saveData();
}