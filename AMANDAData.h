//
// Created by Gabriele Gaetano Fronzé on 01/02/2018.
//

#ifndef MTR_SHUTTLE_AMANDADATA_H
#define MTR_SHUTTLE_AMANDADATA_H

#include <cstdint>

class AMANDAData
{
  public:
    AMANDAData(uint64_t timeStamp=0);

    inline uint64_t getTimeStamp() const { return fTimeStamp; }
    inline void setTimeStamp(uint64_t timeStamp) { AMANDAData::fTimeStamp = timeStamp; }

  private:
    uint64_t fTimeStamp;
};

#endif //MTR_SHUTTLE_AMANDADATA_H
