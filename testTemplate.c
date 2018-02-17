//
// Created by Gabriele Gaetano Fronzé on 06/02/2018.
//

#include "MTRShuttle.h"
#include "RunObject.h"
#include "TGraph.h"

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

  sciattol.loadData("MTR2010_2017_ALL_test.csv");
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

  canv->cd(1);
  auto graph1 = sciattol.drawTrend(&RunObject::getIntCharge,true,true,true,-1,-1,-1,false,&RunObject::isValidForIntCharge);
//  auto graph2 = sciattol.drawTrend(&RunObject::getIntCharge,true,true,false,3,1,5);
//  graph2->SetTitle("MT22 OUT 6");
//  graph2->Draw("ap");
  graph1->Draw("ap");
//  canv->cd(1)->BuildLegend(0.21,0.49,.43,.79,"","P")->SetNColumns(1);
  TLatex aliPerf1;
  aliPerf1.DrawLatexNDC(xpos,ypos,Form("#splitline{ALICE Performance}{      %s}",oss.str().c_str()));

//  canv->Divide(2,2);
//
//  canv->cd(1);
//  auto graph1 = sciattol.drawMaxMin(&RunObject::getIntCharge,true,true,true,0);
//  graph1->Draw("ap");
//  canv->cd(1)->BuildLegend(0.21,0.49,.43,.79,"MT11","P")->SetNColumns(1);
//  TLatex aliPerf1;
//  aliPerf1.DrawLatexNDC(xpos,ypos,Form("#splitline{ALICE Performance}{      %s}",oss.str().c_str()));
//
//  canv->cd(2);
//  auto graph2 = sciattol.drawMaxMin(&RunObject::getIntCharge,true,true,true,1);
//  graph2->Draw("ap");
//  canv->cd(2)->BuildLegend(0.21,0.49,.43,.79,"MT12","P")->SetNColumns(1);
//  TLatex aliPerf2;
//  aliPerf2.DrawLatexNDC(xpos,ypos,Form("#splitline{ALICE Performance}{      %s}",oss.str().c_str()));
//
//  canv->cd(3);
//  auto graph3 = sciattol.drawMaxMin(&RunObject::getIntCharge,true,true,true,2);
//  graph3->Draw("ap");
//  canv->cd(3)->BuildLegend(0.21,0.49,.43,.79,"MT21","P")->SetNColumns(1);
//  TLatex aliPerf3;
//  aliPerf3.DrawLatexNDC(xpos,ypos,Form("#splitline{ALICE Performance}{      %s}",oss.str().c_str()));
//
//  canv->cd(4);
//  auto graph4 = sciattol.drawMaxMin(&RunObject::getIntCharge,true,true,true,3);
//  graph4->Draw("ap");
//  canv->cd(4)->BuildLegend(0.21,0.49,.43,.79,"MT22","P")->SetNColumns(1);
//  TLatex aliPerf4;
//  aliPerf4.DrawLatexNDC(xpos,ypos,Form("#splitline{ALICE Performance}{      %s}",oss.str().c_str()));

  return 0;
}