//
// Created by Gabriele Gaetano Fronzé on 01/02/2018.
//

#ifndef MTR_SHUTTLE_AMANDADATA_H
#define MTR_SHUTTLE_AMANDADATA_H

#include <cstdint>

class AMANDAData
{
  public:
    explicit AMANDAData(uint64_t fTimeStamp=0) = default;

    inline uint64_t getTimeStamp() const { return fTimeStamp; }
    inline void setTimeStamp(uint64_t fTimeStamp) { AMANDAData::fTimeStamp = fTimeStamp; }

  private:
    uint64_t fTimeStamp;
};

#endif //MTR_SHUTTLE_AMANDADATA_H
