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

int testBooster(){
  MTRBooster buster("MTR_2017_test90.csv");

  buster.SetX("Time").SetY("IDark").OnlyDarkCurrentRuns().SetRPC(5).SetSide("OUT").SetPlane(11).StackStage();

  buster.Launch();

  TCanvas *canv_single=new TCanvas("single_plot","single_plot");
  buster.AutoDraw(0,canv_single->cd());



  buster.SetX("Time").SetY("IntCharge").OnlyIntegratedChargeRuns().AccumulateY().PlotAverage().SetPlane(11).SetSide("IN").PlotMinMax().StackStage();
  buster.SetX("Time").SetY("IntCharge").OnlyIntegratedChargeRuns().AccumulateY().PlotAverage().SetPlane(11).SetSide("OUT").PlotMinMax().StackStage();

  buster.SetX("Time").SetY("IntCharge").OnlyIntegratedChargeRuns().AccumulateY().PlotAverage().SetPlane(12).SetSide("IN").PlotMinMax().StackStage();
  buster.SetX("Time").SetY("IntCharge").OnlyIntegratedChargeRuns().AccumulateY().PlotAverage().SetPlane(12).SetSide("OUT").PlotMinMax().StackStage();

  buster.SetX("Time").SetY("IntCharge").OnlyIntegratedChargeRuns().AccumulateY().PlotAverage().SetPlane(21).SetSide("IN").PlotMinMax().StackStage();
  buster.SetX("Time").SetY("IntCharge").OnlyIntegratedChargeRuns().AccumulateY().PlotAverage().SetPlane(21).SetSide("OUT").PlotMinMax().StackStage();

  buster.SetX("Time").SetY("IntCharge").OnlyIntegratedChargeRuns().AccumulateY().PlotAverage().SetPlane(22).SetSide("IN").PlotMinMax().StackStage();
  buster.SetX("Time").SetY("IntCharge").OnlyIntegratedChargeRuns().AccumulateY().PlotAverage().SetPlane(22).SetSide("OUT").PlotMinMax().StackStage();

  buster.Launch();

  TCanvas *canv_multiple=new TCanvas("multiple_plot","multiple_plot");
  canv_multiple->Divide(2,2);

  auto pad1 = canv_multiple->cd(1);
  pad1->Divide(2,1);
  buster.AutoDraw(1,pad1->cd(1),true);
  buster.AutoDraw(2,pad1->cd(2),true);

  auto pad2 = canv_multiple->cd(2);
  pad2->Divide(2,1);
  buster.AutoDraw(3,pad2->cd(1),true);
  buster.AutoDraw(4,pad2->cd(2),true);

  auto pad3 = canv_multiple->cd(3);
  pad3->Divide(2,1);
  buster.AutoDraw(5,pad3->cd(1),true);
  buster.AutoDraw(6,pad3->cd(2),true);

  auto pad4 = canv_multiple->cd(4);
  pad4->Divide(2,1);
  buster.AutoDraw(7,pad4->cd(1),true);
  buster.AutoDraw(8,pad4->cd(2),true);

  return 0;
}