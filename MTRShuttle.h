//
// Created by Gabriele Gaetano Fronzé on 01/02/2018.
//

#ifndef MTR_SHUTTLE_MTRSHUTTLE_H
#define MTR_SHUTTLE_MTRSHUTTLE_H

#include <vector>
#include <string>
#include "RunObject.h"
#include "AMANDACurrent.h"

class MTRShuttle
{
  public:

  private:
    void parseRunList(std::string path="");
    void parseOCDB(std::string path="");
    void parseAMANDA(std::string path="");
    void setIDark();

  private:
    std::vector<int> fRunList;
    std::vector<RunObject> fRunDataVect;
    std::vector<AMANDACurrent> fAMANDACurrentsVect;
};

#endif //MTR_SHUTTLE_MTRSHUTTLE_H
