//
// Created by Gabriele Gaetano Fronzé on 06/02/2018.
//

#include <Riostream.h>
#include <cstdint>
#include <fstream>
#include "MTRShuttle.h"
#include "RunObject.h"
#include "TGraph.h"
#include "TStyle.h"
#include "TCanvas.h"
#include "TLatex.h"
#include "TLegend.h"
#include "boost/any.hpp"

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

int testTemplate(){

  SetStyle(false);

  MTRShuttle sciattol;

  sciattol.loadData("MTRShuttle_2016_2017.csv");
  sciattol.computeAverage();

  TCanvas *canv = new TCanvas("canv","canv");

//  auto graph4 = sciattol.sdrawMaxMin(&RunObject::getIntCharge,true,true,false,0);
//  graph4->Draw("ap");
//  canv->cd()->BuildLegend(0.13,0.8,.30,.89,"Legend","P");


  Double_t xpos=0.5;
  Double_t ypos=0.7;
  std::time_t now = std::time(nullptr);
  std::tm tm = *std::localtime(&now);
  std::ostringstream oss;
  oss << std::put_time(&tm,"%d/%m/%Y");

  auto TS = (uint64_t)1424105780;
  auto TS2017 = (uint64_t)1483291280;

  cond_vector conditions;

  conditions.emplace_back(binder(&RunObject::isAfter,false,TS2017));

  auto graph1 = sciattol.drawTrend(&RunObject::getAvgIDark,false,false,true,-1,-1,-1,conditions);
  auto graph2 = sciattol.drawTrend(&RunObject::getAvgIDark,false,false,false,3,0,2,conditions);
//  graph1->SetLineColor(kBlue);
//  graph2->SetMarkerColor(kBlue);
  graph1->Draw("alp");
  graph2->Draw("lp");

//  canv->cd(1);
//  auto graph1 = sciattol.drawTrend(&RunObject::getAvgIDark,false,false,true,3,0,5,false,&RunObject::isValidForIDark);
//  auto graph2 = sciattol.drawTrend(&RunObject::getAvgITot,false,false,true,3,0,5,false,&RunObject::isValidForIntCharge);
//  graph2->SetLineColor(kBlue);
//  graph2->SetMarkerColor(kBlue);
//
//  auto graph1 = sciattol.drawCorrelations(&RunObject::getEOR,&RunObject::getIntCharge,false,true,true,true,-1,false,&RunObject::getTrue);
//  auto graph2 = sciattol.drawTrend(&RunObject::getIntCharge,true,true,false,3,1,5);
//  graph2->SetTitle("MT22 OUT 6");
//  graph2->Draw("ap");
//  graph1->Draw("ap");
//  graph2->Draw("alp");
//  graph1->Draw("lp");
//  canv->cd(1)->BuildLegend(0.18,0.63,0.35,0.98,"","P")->SetNColumns(1);

  TCanvas *canv1 = new TCanvas("canv1","canv1");

  canv1->Divide(2,2,0,0);

  auto pad1 = canv1->cd(1);
  pad1->Divide(2,1);
  pad1->cd(1);
  auto trends1_in = sciattol.drawTrends(&RunObject::getAvgIDark,false,false,false,0,0,conditions);
  trends1_in->GetYaxis()->SetRangeUser(0.,60.);
  trends1_in->Draw("alp");
  pad1->cd(1)->BuildLegend(0.18,0.63,0.35,0.98,"MT11","P")->SetNColumns(2);

  pad1->cd(2);
  auto trends1_out = sciattol.drawTrends(&RunObject::getAvgIDark,false,false,false,0,1,conditions);
  trends1_out->GetYaxis()->SetRangeUser(0.,60.);
  trends1_out->Draw("alp");
  pad1->cd(2)->BuildLegend(0.18,0.63,0.35,0.98,"MT11","P")->SetNColumns(2);

  auto pad2 = canv1->cd(2);
  pad2->Divide(2,1);
  pad2->cd(1);
  auto trends2_in = sciattol.drawTrends(&RunObject::getAvgIDark,false,false,false,1,0,conditions);
  trends2_in->GetYaxis()->SetRangeUser(0.,60.);
  trends2_in->Draw("alp");
  pad2->cd(1)->BuildLegend(0.18,0.63,0.35,0.98,"MT12","P")->SetNColumns(2);

  pad2->cd(2);
  auto trends2_out = sciattol.drawTrends(&RunObject::getAvgIDark,false,false,false,1,1,conditions);
  trends2_out->GetYaxis()->SetRangeUser(0.,60.);
  trends2_out->Draw("alp");
  pad2->cd(2)->BuildLegend(0.18,0.63,0.35,0.98,"MT12","P")->SetNColumns(2);

  auto pad3 = canv1->cd(3);
  pad3->Divide(2,1);
  pad3->cd(1);
  auto trends3_in = sciattol.drawTrends(&RunObject::getAvgIDark,false,false,false,2,0,conditions);
  trends3_in->GetYaxis()->SetRangeUser(0.,60.);
  trends3_in->Draw("alp");
  pad3->cd(1)->BuildLegend(0.18,0.63,0.35,0.98,"MT21","P")->SetNColumns(2);

  pad3->cd(2);
  auto trends3_out = sciattol.drawTrends(&RunObject::getAvgIDark,false,false,false,2,1,conditions);
  trends3_out->GetYaxis()->SetRangeUser(0.,60.);
  trends3_out->Draw("alp");
  pad3->cd(2)->BuildLegend(0.18,0.63,0.35,0.98,"MT21","P")->SetNColumns(2);

  auto pad4 = canv1->cd(4);
  pad4->Divide(2,1);
  pad4->cd(1);
  auto trends4_in = sciattol.drawTrends(&RunObject::getAvgIDark,false,false,false,3,0,conditions);
  trends4_in->GetYaxis()->SetRangeUser(0.,60.);
  trends4_in->Draw("alp");
  pad4->cd(1)->BuildLegend(0.18,0.63,0.35,0.98,"MT22","P")->SetNColumns(2);

  pad4->cd(2);
  auto trends4_out = sciattol.drawTrends(&RunObject::getAvgIDark,false,false,false,3,1,conditions);
  trends4_out->GetYaxis()->SetRangeUser(0.,60.);
  trends4_out->Draw("alp");
  pad4->cd(2)->BuildLegend(0.18,0.63,0.35,0.98,"MT22","P")->SetNColumns(2);

  return 0;
}