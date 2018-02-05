//
// Created by Gabriele Gaetano Fronzé on 01/02/2018.
//

#ifndef MTR_SHUTTLE_MTRSHUTTLE_H
#define MTR_SHUTTLE_MTRSHUTTLE_H

#include <vector>
#include <string>
#include "RunObject.h"
#include "AMANDACurrent.h"
#include "Parameters.h"

class MTRShuttle
{
  public:

  private:
    void parseRunList(std::string path="");
    void parseOCDB(std::string path="");
    void parseAMANDAiMon(std::string path = "");
    void parseAMANDAVeff(std::string path = "");
    void setIDark();

  private:
    std::vector<int[2]> fRunList;
    std::vector<RunObject> fRunDataVect[kNPlanes][kNSides][kNRPC];
    std::vector<AMANDACurrent> fAMANDACurrentsVect[kNPlanes][kNSides][kNRPC];

    inline double getM(const AMANDACurrent iStart, const AMANDACurrent iStop) {
      return (iStop.getIDark()-iStart.getIDark())/(iStop.getTimeStamp()-iStart.getTimeStamp());
    };

    inline double getQ(const AMANDACurrent iStart, const AMANDACurrent iStop) {
      return iStart.getIDark();
    };
};

#endif //MTR_SHUTTLE_MTRSHUTTLE_H
