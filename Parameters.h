//
// Created by Gabriele Gaetano Fronzé on 01/02/2018.
//

#ifndef MTR_SHUTTLE_PARAMETERS_H
#define MTR_SHUTTLE_PARAMETERS_H

#include <cstdint>
#include <string>
#include <Rtypes.h>

// Geometry and segmentation
static const int kNSides=2;
static const int kNPlanes=4;
static const int kNRPC=9;
static const int kNLocalBoards=234;
static const int kNCathodes=2;

// Naming arrays to read AMANDA and create plot titles
static const std::string kSides[]={"INSIDE","OUTSIDE"};
static const std::string kSidesShort[]={"IN","OUT"};
static const int kPlanes[]={11,12,21,22};
static const std::string kCathodes[]={"BENDING","NOT BENDING"};

// Arrays to convert iRPC={0,17} into iRPC={1,9}x{inside,outside}
static const int kRPCIndexes[] = {5,6,7,8,9,9,8,7,6,5,4,3,2,1,1,2,3,4};
static const int kRPCSides[] = {0,0,0,0,0,1,1,1,1,1,1,1,1,1,0,0,0,0};
static const int kLBToRPC[][3] = {{1,1,0},{2,2,0},{3,2,0},{4,3,0},{5,3,0},{6,4,0},{7,4,0},{8,4,0},{9,6,0},{10,6,0},{11,6,0},
                               {12,7,0},{13,7,0},{14,8,0},{15,8,0},{16,9,0},{17,1,0},{18,2,0},{19,2,0},{20,3,0},{21,3,0},
                               {22,4,0},{23,4,0},{24,4,0},{25,4,0},{26,5,0},{27,5,0},{28,5,0},{29,5,0},{30,6,0},{31,6,0},
                               {32,6,0},{33,6,0},{34,7,0},{35,7,0},{36,8,0},{37,8,0},{38,9,0},{39,1,0},{40,2,0},{41,2,0},
                               {42,3,0},{43,3,0},{44,4,0},{45,4,0},{46,4,0},{47,4,0},{48,5,0},{49,5,0},{50,5,0},{51,5,0},
                               {52,6,0},{53,6,0},{54,6,0},{55,6,0},{56,7,0},{57,7,0},{58,8,0},{59,8,0},{60,9,0},{61,1,0},
                               {62,2,0},{63,2,0},{64,3,0},{65,3,0},{66,4,0},{67,4,0},{68,5,0},{69,5,0},{70,6,0},{71,6,0},
                               {72,7,0},{73,7,0},{74,8,0},{75,8,0},{76,9,0},{77,1,0},{78,2,0},{79,2,0},{80,3,0},{81,3,0},
                               {82,4,0},{83,4,0},{84,5,0},{85,5,0},{86,6,0},{87,6,0},{88,7,0},{89,7,0},{90,8,0},{91,8,0},
                               {92,9,0},{93,1,0},{94,2,0},{95,2,0},{96,3,0},{97,3,0},{98,4,0},{99,4,0},{100,5,0},{101,5,0},
                               {102,6,0},{103,6,0},{104,7,0},{105,7,0},{106,8,0},{107,8,0},{108,9,0},{109,1,0},{110,2,0},
                               {111,3,0},{112,4,0},{113,5,0},{114,6,0},{115,7,0},{116,8,0},{117,9,0},{118,1,1},{119,2,1},
                               {120,2,1},{121,3,1},{122,3,1},{123,4,1},{124,4,1},{125,4,1},{126,6,1},{127,6,1},{128,6,1},
                               {129,7,1},{130,7,1},{131,8,1},{132,8,1},{133,9,1},{134,1,1},{135,2,1},{136,2,1},{137,3,1},
                               {138,3,1},{139,4,1},{140,4,1},{141,4,1},{142,4,1},{143,5,1},{144,5,1},{145,5,1},{146,5,1},
                               {147,6,1},{148,6,1},{149,6,1},{150,6,1},{151,7,1},{152,7,1},{153,8,1},{154,8,1},{155,9,1},
                               {156,1,1},{157,2,1},{158,2,1},{159,3,1},{160,3,1},{161,4,1},{162,4,1},{163,4,1},{164,4,1},
                               {165,5,1},{166,5,1},{167,5,1},{168,5,1},{169,6,1},{170,6,1},{171,6,1},{172,6,1},{173,7,1},
                               {174,7,1},{175,8,1},{176,8,1},{177,9,1},{178,1,1},{179,2,1},{180,2,1},{181,3,1},{182,3,1},
                               {183,4,1},{184,4,1},{185,5,1},{186,5,1},{187,6,1},{188,6,1},{189,7,1},{190,7,1},{191,8,1},
                               {192,8,1},{193,9,1},{194,1,1},{195,2,1},{196,2,1},{197,3,1},{198,3,1},{199,4,1},{200,4,1},
                               {201,5,1},{202,5,1},{203,6,1},{204,6,1},{205,7,1},{206,7,1},{207,8,1},{208,8,1},{209,9,1},
                               {210,1,1},{211,2,1},{212,2,1},{213,3,1},{214,3,1},{215,4,1},{216,4,1},{217,5,1},{218,5,1},
                               {219,6,1},{220,6,1},{221,7,1},{222,7,1},{223,8,1},{224,8,1},{225,9,1},{226,1,1},{227,2,1},
                               {228,3,1},{229,4,1},{230,5,1},{231,6,1},{232,7,1},{233,8,1},{234,9,1}};
static const double kAreas[4][2][9] = {{{ 17340.000000, 17340.000000, 17340.000000, 16762.000000, 13872.000000, 16762.000000, 17340.000000, 17340.000000, 17340.000000 },
                                        { 17340.000000, 17340.000000, 17340.000000, 16762.000000, 13872.000000, 16762.000000, 17340.000000, 17340.000000, 17340.000000 }},
                                       {{ 17709.556322, 17709.556322, 17709.556322, 17119.237778, 14167.645058, 17119.237778, 17709.556322, 17709.556322, 17709.556322 },
                                        { 17709.556322, 17709.556322, 17709.556322, 17119.237778, 14167.645058, 17119.237778, 17709.556322, 17709.556322, 17709.556322 }},
                                       {{ 19570.076065, 19570.076065, 19570.076065, 18917.740196, 15656.060852, 18917.740196, 19570.076065, 19570.076065, 19570.076065 },
                                        { 19570.076065, 19570.076065, 19570.076065, 18917.740196, 15656.060852, 18917.740196, 19570.076065, 19570.076065, 19570.076065 }},
                                       {{ 19962.556422, 19962.556422, 19962.556422, 19297.137875, 15970.045138, 19297.137875, 19962.556422, 19962.556422, 19962.556422 },
                                        { 19962.556422, 19962.556422, 19962.556422, 19297.137875, 15970.045138, 19297.137875, 19962.556422, 19962.556422, 19962.556422 }}};

// Detector parameters
static const double kMinWorkHV=10000.;
static const uint64_t kFullScale=65535;

//Graph maquillage vectors
static const Color_t kColors[] ={kBlue+2,kRed,kGreen,kBlue,kViolet+7,kMagenta,kCyan,kGray,kOrange};
static const Style_t kMarkers[]={20,24,21,25};

//double fRPCAreas[kNRPC][kNPlanes];
//double fLBAreas[kNLocalBoards][kNPlanes];
//double fTinyArea[kNPlanes];
//double fLittleArea[kNPlanes];
//double fNormalArea[kNPlanes];
//double fBigArea[kNPlanes];
//
//
//static const std::string fSides_[] = {"INSIDE","OUTSIDE"};
//const std::string *fSides = fSides_;
//static const int fPlanes_[] = {11,12,21,22};
//const int *fPlanes = fPlanes_;
//static const std::string fCathodes_[] = {"BENDING","NOT BENDING"};
//const std::string *fCathodes = fCathodes_;
//
//
//const int *fColors = fColors_;
//static const int fStyles_[]={20,24,21,25};
//const int *fStyles = fStyles_;
//
////call this in constructors
//void InitDataMembers(){
//  for(int iRPC=0;iRPC<kNRPC;iRPC++){
//    for(int iPlane=0;iPlane<kNPlanes;iPlane++){
//      if(iRPC==5){
//        if(iPlane<2){
//          fRPCAreas[iRPC][iPlane]=16056.;
//        } else {
//          fRPCAreas[iRPC][iPlane]=18176.;
//        }
//      }else if(iRPC==4 || iRPC==6){
//        if(iPlane<2){
//          fRPCAreas[iRPC][iPlane]=19728./28.*27.;
//        } else {
//          fRPCAreas[iRPC][iPlane]=22338./28.*27.;
//        }
//      }else {
//        if(iPlane<2){
//          fRPCAreas[iRPC][iPlane]=19728.;
//        } else {
//          fRPCAreas[iRPC][iPlane]=22338.;
//        }
//      }
//    }
//  }
//
//  for(int iPlane=0;iPlane<kNPlanes;iPlane++){
//    fTinyArea[iPlane]=(fRPCAreas[9-1][iPlane]/7.*6.-fRPCAreas[5-1][iPlane])/4.;
//    fLittleArea[iPlane]=fRPCAreas[9-1][iPlane]/28.;
//    fNormalArea[iPlane]=fRPCAreas[9-1][iPlane]/14.;
//    fBigArea[iPlane]=fRPCAreas[9-1][iPlane]/7.;
//  }
//
//  for(int iLB=0;iLB<kNLocalBoards;iLB++){
//    for(int iPlane=0;iPlane<kNPlanes;iPlane++){
//      if((iLB>=25 && iLB<=28) || (iLB>=142 && iLB<=145)){
//        fLBAreas[iLB][iPlane]=fTinyArea[iPlane];
//      } else if((iLB>=43 && iLB<=54) || (iLB>=21 && iLB<=24) || (iLB>=29 && iLB<=32) || (iLB>=5 && iLB<=10) || (iLB>=122 && iLB<=127) || (iLB>=138 && iLB<=141) || (iLB>=146 && iLB<=149) || (iLB>=160 && iLB<=171)){
//        fLBAreas[iLB][iPlane]=fLittleArea[iPlane];
//      } else if((iLB>=108 && iLB<=116) || (iLB>=225 && iLB<=233) || iLB==0 || iLB==16 || iLB==38 || iLB==60 || iLB==76 || iLB==92 || iLB==117 || iLB==133 || iLB==155 || iLB==177 || iLB==193 || iLB==209 || iLB==224 || iLB==208 || iLB==192 || iLB==176 || iLB==154 || iLB==132 || iLB==15 || iLB==37 || iLB==59 || iLB==75 || iLB==91 || iLB==107){
//        fLBAreas[iLB][iPlane]=fBigArea[iPlane];
//      } else {
//        fLBAreas[iLB][iPlane]=fNormalArea[iPlane];
//      }
//
//      //cout<<iLB+1<<" "<<LBAreas[iLB][iPlane]<<endl;
//    }
//  }
//}

#endif //MTR_SHUTTLE_PARAMETERS_H
