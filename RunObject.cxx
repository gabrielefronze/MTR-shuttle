//
// Created by Gabriele Gaetano Fronzé on 01/02/2018.
//

#include "RunObject.h"

RunObject::RunObject(uint64_t fSOR, uint64_t fEOR, double fAvgHV, double fAvgITot, double fAvgIDark, double fIntCharge,
                     uint64_t fScalersBending, uint64_t fScalersNotBending) : fSOR(fSOR),
                                                                              fEOR(fEOR),
                                                                              fAvgHV(fAvgHV),
                                                                              fAvgITot(fAvgITot),
                                                                              fAvgIDark(fAvgIDark),
                                                                              fIntCharge(fIntCharge),
                                                                              fScalersBending(fScalersBending),
                                                                              fScalersNotBending(fScalersNotBending)
{}
