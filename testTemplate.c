//
// Created by Gabriele Gaetano Fronzé on 06/02/2018.
//

#include "MTRShuttle.h"
#include "RunObject.h"
#include "TGraph.h"

int testTemplate(){
  MTRShuttle sciattol;

  sciattol.loadData("MTRShuttle.csv");
  sciattol.computeAverage();

  auto graph = sciattol.drawTrends(&RunObject::getIntCharge,true,true,true);
  graph->Draw("ap");

//  TCanvas *canv = new TCanvas("canv","canv");
//  canv->Divide(1,2);

//  canv->cd(1);
//  auto graph = sciattol.drawTrends(&RunObject::getScalNotBending,true,false,false,&RunObject::isDark,true);
//  graph->Draw("ap");

//  canv->cd(2);
//  auto graph1 = sciattol.drawTrends(&RunObject::getScalNotBending,true,false,false,&RunObject::isDark,false);
//  graph1->Draw("ap");
//  gStyle->SetTimeOffset(0);
//  graph->GetXaxis()->SetTimeDisplay(1);
//  graph->GetXaxis()->SetTimeFormat("%d\/%m\/%Y");
//  graph->GetXaxis()->SetLabelSize(0.03);
//  gPad->Update();
//  gPad->Modified();


  return 0;
}