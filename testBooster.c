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

int testBooster(){
//  MTRBooster buster("test_Diego_flat_dummy.csv");
//  MTRBooster buster("test_Diego_flat_dummy.csv");
//  MTRBooster buster("test_Diego_weighted_dummy.csv");
//  MTRBooster buster("testALL.csv");

  std::list<std::string> runLists{"/Users/Gabriele/cernbox/Dottorato/MTR2017/runsCOSMIC1018.txt","/Users/Gabriele/cernbox/Dottorato/MTR2017/runs_2010_2017_PHYSICS"};

  MTRShuttle sciattol;
  sciattol.instance(runLists, "Imon1018.txt", "vMon1018.txt","local:///Users/Gabriele/cernbox/Dottorato/MTR2017/CDB/####/OCDB");

  MTRBooster buster(sciattol);

  buster.LoadReplacedRPCs("testrep.txt");

  buster.SetX("Time").SetY("IntCharge").SetPlane(12).NormalizeToArea().AccumulateY().PlotAverage().PlotMinMax().StackStage();
  // buster.SetX("Time").SetY("RateBend").SetPlane(22).SetSide("IN").SetRPC(3).OnlyWithBeamRuns().StackStage();
  buster.SetX("Time").SetY("RateBend").SetPlane(22).SetSide("IN").SetRPC(3).OnlyNoBeamRuns().StackStage();
  // buster.SetX("Time").SetY("IntCharge").OnlyIntegratedChargeRuns().SetPlane(22).SetSide("IN").SetRPC(3).NormalizeToArea().AccumulateY().PlotAverage().StackStage();
//  buster.SetX("Time").SetY("IntCharge").AccumulateY().NormalizeToArea().OnlyIntegratedChargeRuns().PlotAverage().StackStage();

std::cout << "Launch!!!" << std::endl;

  buster.Launch();

  TCanvas *canv_single=new TCanvas("single_plot","single_plot");
  canv_single->Divide(2,1);
  buster.AutoDraw(0,canv_single->cd(1),true);
  buster.AutoDraw(1,canv_single->cd(2),true);

//   buster.SetX("Time").SetY("IntCharge").SetPlane(11).NormalizeToArea().AccumulateY().PlotAverage().StackStage();
// //  buster.SetX("Time").SetY("IntCharge").OnlyIntegratedChargeRuns().SetPlane(11).NormalizeToArea().AccumulateY().PlotAverage().StackStage();

//   buster.SetX("Time").SetY("IntCharge").SetPlane(12).NormalizeToArea().AccumulateY().PlotAverage().StackStage();
// //  buster.SetX("Time").SetY("IntCharge").OnlyIntegratedChargeRuns().SetPlane(12).NormalizeToArea().AccumulateY().PlotAverage().StackStage();

//   buster.SetX("Time").SetY("IntCharge").SetPlane(21).NormalizeToArea().AccumulateY().PlotAverage().StackStage();
// //  buster.SetX("Time").SetY("IntCharge").OnlyIntegratedChargeRuns().SetPlane(21).NormalizeToArea().AccumulateY().PlotAverage().StackStage();

//   buster.SetX("Time").SetY("IntCharge").SetPlane(22).NormalizeToArea().AccumulateY().PlotAverage().StackStage();
// //  buster.SetX("Time").SetY("IntCharge").OnlyIntegratedChargeRuns().SetPlane(22).NormalizeToArea().AccumulateY().PlotAverage().StackStage();

//   buster.Launch();

//   TCanvas *canv_multiple=new TCanvas("multiple_plot","multiple_plot");
//   canv_multiple->Divide(2,2);

//   auto pad1 = canv_multiple->cd(1);
// //  pad1->Divide(2,1);
// //  buster.AutoDraw(1,pad1->cd(1),true);
// //  buster.AutoDraw(2,pad1->cd(2),true);
//   buster.AutoDraw(1,pad1->cd(),true);

//   auto pad2 = canv_multiple->cd(2);
// //  pad2->Divide(2,1);
// //  buster.AutoDraw(3,pad2->cd(1),true);
// //  buster.AutoDraw(4,pad2->cd(2),true);
//   buster.AutoDraw(2,pad2->cd(),true);

//   auto pad3 = canv_multiple->cd(3);
// //  pad3->Divide(2,1);
// //  buster.AutoDraw(5,pad3->cd(1),true);
// //  buster.AutoDraw(6,pad3->cd(2),true);
//   buster.AutoDraw(3,pad3->cd(),true);

//   auto pad4 = canv_multiple->cd(4);
// //  pad4->Divide(2,1);
// //  buster.AutoDraw(7,pad4->cd(1),true);
// //  buster.AutoDraw(8,pad4->cd(2),true);
//   buster.AutoDraw(4,pad4->cd(),true);

  return 0;
}