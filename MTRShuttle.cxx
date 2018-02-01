//
// Created by Gabriele Gaetano Fronzé on 01/02/2018.
//

#include <iostream>
#include <fstream>
#include "MTRShuttle.h"

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

}

void MTRShuttle::parseAMANDA(std::string path)
{
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
      fAMANDACurrentsVect.push_back(bufferCurrent);
    }
    std::cout<<std::endl;
    fin.close();
  }
  else std::cout << "Unable to open file";

  std::cout << "Loaded " << << "AMANDA values" << std::endl;
}
