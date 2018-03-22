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

void SetStyle(Bool_t graypalette) {
  cout << "Setting style!" << endl;

  gStyle->Reset("Plain");
  gStyle->SetOptTitle(0);
  gStyle->SetOptStat(0);
  if(graypalette) gStyle->SetPalette(8,0);
  else gStyle->SetPalette(1);
  gStyle->SetCanvasColor(10);
  gStyle->SetCanvasBorderMode(0);
  gStyle->SetFrameLineWidth(1);
  gStyle->SetFrameFillColor(kWhite);
  gStyle->SetPadColor(10);
  gStyle->SetPadTickX(1);
  gStyle->SetPadTickY(1);
  gStyle->SetPadBottomMargin(0.15);
  gStyle->SetPadLeftMargin(0.15);
  gStyle->SetHistLineWidth(1);
  gStyle->SetHistLineColor(kRed);
  gStyle->SetFuncWidth(2);
  gStyle->SetFuncColor(kGreen);
  gStyle->SetLineWidth(2);
  gStyle->SetLabelSize(0.045,"xyz");
  gStyle->SetLabelOffset(0.01,"y");
  gStyle->SetLabelOffset(0.01,"x");
  gStyle->SetLabelColor(kBlack,"xyz");
  gStyle->SetTitleSize(0.05,"xyz");
  gStyle->SetTitleOffset(1.25,"y");
  gStyle->SetTitleOffset(1.2,"x");
  gStyle->SetTitleFillColor(kWhite);
  gStyle->SetTextSizePixels(26);
  gStyle->SetTextFont(42);
  //  gStyle->SetTickLength(0.04,"X");  gStyle->SetTickLength(0.04,"Y");

  gStyle->SetLegendBorderSize(0);
  gStyle->SetLegendFillColor(kWhite);
  //  gStyle->SetFillColor(kWhite);
  gStyle->SetLegendFont(42);


}

int testBooster(){

//  SetStyle(false);

  MTRBooster buster("MTR_2017_test90.csv");

  buster.SetX("Time")
        .SetY("IntCharge")
        .AccumulateY()
        .OnlyIntegratedChargeRuns()
        .SetRPC(5)
        .SetSide("OUT")
        .SetPlane(11)
        .StackStage();

  buster.SetX("Time").SetY("IDark").OnlyDarkCurrentRuns().SetPlane(11).SetSide("IN").StackStage();
  buster.SetX("Time").SetY("IDark").OnlyDarkCurrentRuns().SetPlane(11).SetSide("OUT").StackStage();

  buster.SetX("Time").SetY("IDark").OnlyDarkCurrentRuns().SetPlane(12).SetSide("IN").StackStage();
  buster.SetX("Time").SetY("IDark").OnlyDarkCurrentRuns().SetPlane(12).SetSide("OUT").StackStage();

  buster.SetX("Time").SetY("IDark").OnlyDarkCurrentRuns().SetPlane(21).SetSide("IN").StackStage();
  buster.SetX("Time").SetY("IDark").OnlyDarkCurrentRuns().SetPlane(21).SetSide("OUT").StackStage();

  buster.SetX("Time").SetY("IDark").OnlyDarkCurrentRuns().SetPlane(22).SetSide("IN").StackStage();
  buster.SetX("Time").SetY("IDark").OnlyDarkCurrentRuns().SetPlane(22).SetSide("OUT").StackStage();

  buster.Launch();

  TCanvas *canv_single=new TCanvas("single_plot","single_plot");
  buster.AutoDraw(0,canv_single->cd(),"alp");

  TCanvas *canv_multiple=new TCanvas("multiple_plot","multiple_plot");
  canv_multiple->Divide(2,2);

  auto pad1 = canv_multiple->cd(1);
  pad1->Divide(2,1);
  buster.AutoDraw(1,pad1->cd(1),"alp",true);
  buster.AutoDraw(2,pad1->cd(2),"alp",true);


  auto pad2 = canv_multiple->cd(2);
  pad2->Divide(2,1);
  buster.AutoDraw(3,pad2->cd(1),"alp",true);
  buster.AutoDraw(4,pad2->cd(2),"alp",true);


  auto pad3 = canv_multiple->cd(3);
  pad3->Divide(2,1);
  buster.AutoDraw(5,pad3->cd(1),"alp",true);
  buster.AutoDraw(6,pad3->cd(2),"alp",true);


  auto pad4 = canv_multiple->cd(4);
  pad4->Divide(2,1);
  buster.AutoDraw(7,pad4->cd(1),"alp",true);
  buster.AutoDraw(8,pad4->cd(2),"alp",true);



  return 0;
}