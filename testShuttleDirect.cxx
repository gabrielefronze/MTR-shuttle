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

int testShuttleDirect()
{
  MTRShuttle sciattol;
  sciattol.instance("runListDiego.txt", "Imon1018.txt", "vMon1018.txt",
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

  uint iPoint = 0;
  double cumulus=0.;
  for (const auto &itRun : sciattol.fRunDataVect) {

    cumulus+=itRun.getIntCharge(kMT11,kINSIDE,k1);
    intChargeGr->SetPoint(iPoint,itRun.getEOR(),cumulus);

    iDarkGr->SetPoint(iPoint,itRun.getEOR(),itRun.getAvgIDark(kMT11,kINSIDE,k1));

    iTotGr->SetPoint(iPoint,itRun.getEOR(),itRun.getAvgITot(kMT11,kINSIDE,k1));

    HVGr->SetPoint(iPoint,itRun.getEOR(),itRun.getAvgHV(kMT11,kINSIDE,k1));

    printf("%llu,%llu,%f,%f,%f,%f\n",itRun.getRunNumber(),itRun.getEOR(),cumulus,itRun.getAvgIDark(kMT11,kINSIDE,k1),itRun.getAvgITot(kMT11,kINSIDE,k1),itRun.getAvgHV(kMT11,kINSIDE,k1));

    iPoint++;
  }

  auto mg = new TMultiGraph();
  mg->Add(intChargeGr);
  mg->Add(iDarkGr);
  mg->Add(iTotGr);
  mg->Add(HVGr);

  mg->Draw("alp");

  return 0;
}