//
// Created by Gabriele Gaetano Fronzé on 06/02/2018.
//

#include <Riostream.h>

#ifdef APPLE
#include <_types/_uint8_t.h>
#include <_types/_uint16_t.h>
#include <_types/_uint32_t.h>
#include <_types/_uint64_t.h>
#else
#include <stdint.h>
#endif

#include <fstream>
#include "MTRBooster.h"
#include "RunObject.h"
#include "TGraph.h"
#include "TStyle.h"
#include "TCanvas.h"
#include "TLatex.h"
#include "TLegend.h"
#include "boost/any.hpp"
#include "MTRConditions.h"
#include "MTRShuttle.h"

int testAMANDA()
{

  uint64_t minTS = 1470355200-1;
  uint64_t maxTS = 1470700800+1;

  MTRShuttle sciattol;
  sciattol.instance("runlist16Ltest.txt", "Imon1018.txt", "vMon1018.txt",
                    "local:///Users/Gabriele/cernbox/Dottorato/MTR2017/CDB/####/OCDB");

  auto *intChargeGr = new TGraph();
  auto *iDarkGr = new TGraph();
  auto *iTotGr = new TGraph();
  auto *HVGr = new TGraph();

  intChargeGr->SetLineColor(kGreen);
  intChargeGr->SetMarkerColor(kGreen);
  intChargeGr->GetXaxis()->SetTimeDisplay(1);
  intChargeGr->GetXaxis()->SetTimeFormat("%d/%m/%y");

  iDarkGr->SetLineColor(kRed);
  iDarkGr->SetMarkerColor(kRed);
  iDarkGr->GetXaxis()->SetTimeDisplay(1);
  iDarkGr->GetXaxis()->SetTimeFormat("%d/%m/%y");

  iTotGr->SetLineColor(kBlue);
  iTotGr->SetMarkerColor(kBlue);
  iTotGr->GetXaxis()->SetTimeDisplay(1);
  iTotGr->GetXaxis()->SetTimeFormat("%d/%m/%y");

  HVGr->SetLineColor(kViolet);
  HVGr->SetMarkerColor(kViolet);
  HVGr->GetXaxis()->SetTimeDisplay(1);
  HVGr->GetXaxis()->SetTimeFormat("%d/%m/%y");

  for(const auto &itRun : sciattol.fRunDataVect){
    printf("%llu,%llu,%llu %d %d\n",itRun.getRunNumber(),itRun.getSOR(),itRun.getEOR(),itRun.isDark(), itRun.isDummy());
  }

//  for(const auto &itCurrent : sciattol.fAMANDACurrentsVect[kMT22][kINSIDE][k2]){
//    if(itCurrent.getTimeStamp()>minTS && itCurrent.getTimeStamp()<maxTS)
//      printf("%llu,%f,%f,%d\n",itCurrent.getTimeStamp(),itCurrent.getITot(),itCurrent.getIDark(),itCurrent.isDark());
//  }
//
//  printf("\n###########\n");
//
//  for(const auto &itVoltage : sciattol.fAMANDAVoltagesVect[kMT22][kINSIDE][k2]){
//    if(itVoltage.getTimeStamp()>minTS && itVoltage.getTimeStamp()<maxTS)
//      printf("%llu,%f\n",itVoltage.getTimeStamp(),itVoltage.getHV());
//  }

  return 0;
}