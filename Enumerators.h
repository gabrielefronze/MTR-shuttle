//
// Created by Gabriele Gaetano Fronzé on 20/03/2018.
//

#ifndef MTR_SHUTTLE_ENUMERATORS_H
#define MTR_SHUTTLE_ENUMERATORS_H

enum MTRPlanes{
    kMT11,
    kMT12,
    kMT21,
    kMT22,
    kNPlanes
};

enum MTRSides{
    kINSIDE,
    kOUTSIDE,
    kNSides
};

enum MTRRPCs{
    k1,
    k2,
    k3,
    k4,
    k5,
    k6,
    k7,
    k8,
    k9,
    kNRPCs
};

static const int kNLocalBoards=234;
static const int kNCathodes=2;

#endif //MTR_SHUTTLE_ENUMERATORS_H
