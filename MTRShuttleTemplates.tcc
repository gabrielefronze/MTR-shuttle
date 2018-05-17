//
// Created by Gabriele Gaetano Fronzé on 20/03/2018.
//

#include "MTRShuttle.h"

template<typename XType, typename YType>
TGraph *drawCorrelation(XType (RunObject::*getX)() const,
                        YType (RunObject::*getY)() const,
                        bool normalizeToAreaX=false,
                        bool normalizeToAreaY=false,
                        bool accumulate=false,
                        bool plotAverage=false,
                        MTRPlanes plane=MTRPlanes::kAll,
                        MTRSides side=MTRSides::kBoth,
                        MTRRPCs RPC=MTRRPCs::kAllRPCs,
                        MTRConditions *conditions=new MTRConditions())
{
  auto *returnedGraph = new TGraph();
  if(!plotAverage){
    if (plane<MTRPlanes::kNPlanes) returnedGraph->SetNameTitle(Form("%d_%d_%d",plane,side,RPC+1),Form("MT%d %s %d",kPlanes[plane],kSidesShort[side].c_str(),RPC+1));
    else returnedGraph->SetNameTitle(Form("%d_%d",side,RPC+1),Form("%s %d",kSidesShort[side].c_str(),RPC+1));
  } else {
    if( plane<MTRPlanes::kNPlanes) returnedGraph->SetNameTitle(Form("avg_%d",kPlanes[plane]),Form("Average MT%d",kPlanes[plane]));
    else returnedGraph->SetNameTitle("avg","Average");
  }
  graphMaquillage(plane, RPC, returnedGraph, plotAverage);

  returnedGraph->GetXaxis()->SetTitle(generateLabel(getX, normalizeToAreaX).c_str());
  returnedGraph->GetYaxis()->SetTitle(generateLabel(getY, normalizeToAreaY).c_str());

  if (funcCmp(getX, &RunObject::getSOR) || funcCmp(getX, &RunObject::getEOR)){
    //This time offset is NEEDED to correctly display data from timestamp!
    gStyle->SetTimeOffset(0);
    returnedGraph->GetXaxis()->SetTimeDisplay(1);
    returnedGraph->GetXaxis()->SetTimeFormat("%d-%m-%y");
    returnedGraph->GetXaxis()->SetLabelSize(0.02);
    returnedGraph->GetXaxis()->SetTitle("Date");
  }

  if (funcCmp(getY, &RunObject::getSOR) || funcCmp(getY, &RunObject::getEOR)){
    //This time offset is NEEDED to correctly display data from timestamp!
    gStyle->SetTimeOffset(0);
    returnedGraph->GetYaxis()->SetTimeDisplay(1);
    returnedGraph->GetYaxis()->SetTimeFormat("%d-%m-%y");
    returnedGraph->GetYaxis()->SetLabelSize(0.02);
    returnedGraph->GetYaxis()->SetTitle("Date");
  }

  int counter = 0;

  auto yCumulus = (YType)0.;//(YType)4000.;

  auto dataVector = (!plotAverage)?fRunDataVect[plane][side][RPC]:fRunDataVectAvg[(plane<MTRPlanes::kNPlanes)?plane:4];

  bool resetMT12OUTSIDE6 = true;
  bool isReplacedRPC = ( plane==1
                         && side==1
                         && RPC==5
                         && funcCmp(getY, &RunObject::getIntCharge)
                         && accumulate );

  for( auto const &dataIt : dataVector){

    bool shouldPlot = true;
    for (const auto &itCondition : (*conditions)()) {
      shouldPlot &= itCondition(&dataIt);
    }
    if (!shouldPlot) continue;

    XType x = (dataIt.*getX)();
    YType y = (dataIt.*getY)();

    if ( std::isnan(y) || std::isnan(x) ) continue;

    if (normalizeToAreaX){
      if(!plotAverage) x=x/kAreas[plane][side][RPC];
      else x=x/kAreas[0][0][0];
    }

    if (normalizeToAreaY){
      if(!plotAverage) y=y/kAreas[plane][side][RPC];
      else y=y/kAreas[0][0][0];
    }

    if ( y==(YType)0 ) continue;

    if ( isReplacedRPC && resetMT12OUTSIDE6 && dataIt.getSOR()>1477958400 ){
      resetMT12OUTSIDE6 = false;
      yCumulus = (YType)0;
    }
    returnedGraph->SetPoint(counter++,(double)x,(double)((accumulate)?(yCumulus+=y):y));
  }

  return returnedGraph;
}

template<typename XType, typename YType, class ...Args>
TGraph *drawCorrelation(XType (RunObject::*getX)() const,
                                    YType (RunObject::*getY)() const,
                                    bool normalizeToAreaX,
                                    bool normalizeToAreaY,
                                    bool accumulate,
                                    bool plotAverage,
                                    MTRPlanes plane,
                                    MTRSides side,
                                    MTRRPCs RPC,
                                    bool (RunObject::*condition)(Args...) const,
                                    bool negate,
                                    Args... args){
  MTRConditions dummyCond;
  dummyCond.addCondition(condition,negate,args...);
  return drawCorrelation(getX,
                         getY,
                         normalizeToAreaX,
                         normalizeToAreaY,
                         accumulate,
                         plotAverage,
                         plane,
                         side,
                         RPC,
                         &dummyCond);
}

template<typename XType, typename YType, typename CondType>
TMultiGraph *drawCorrelations(XType(RunObject::*getX)() const,
                              YType(RunObject::*getY)() const,
                              bool normalizeToAreaX=false,
                              bool normalizeToAreaY=false,
                              bool accumulate=false,
                              bool plotAverage=false,
                              MTRPlanes plane=MTRPlanes::kAll,
                              MTRSides side=MTRSides::kBoth,
                              CondType *conditions=nullptr)
{
  auto *mg = new TMultiGraph();

  for (int iPlane=MTRPlanes::kMT11; iPlane<MTRPlanes::kNPlanes; iPlane++) {
    if ( plane!=MTRPlanes::kAll && iPlane!=plane ) continue;
    for (int iSide =MTRSides::kINSIDE; iSide < MTRSides::kNSides; iSide++) {
      if ( side!=MTRSides::kBoth && iSide!=side ) continue;
      for (int iRPC=MTRRPCs::k1; iRPC < MTRRPCs::kNRPCs; iRPC++) {
        mg->Add(
          drawCorrelation(getX,
                          getY,
                          normalizeToAreaX,
                          normalizeToAreaY,
                          accumulate,
                          false,
                          (MTRPlanes)iPlane,
                          (MTRSides)iSide,
                          (MTRRPCs)iRPC,
                          conditions));
      }
    }
  }

  if (plotAverage) {
    mg->Add(
      drawCorrelation(getX,
                      getY,
                      normalizeToAreaX,
                      normalizeToAreaY,
                      accumulate,
                      plotAverage,
                      plane,
                      MTRSides::kBoth,
                      kAllRPCs,
                      conditions));
  }

  if(!(mg->GetListOfGraphs())) return nullptr;

//  mg->Draw("ap");
//
//  if (funcCmp(getX, &RunObject::getSOR) || funcCmp(getX, &RunObject::getEOR)){
//    //This time offset is NEEDED to correctly display data from timestamp!
//    gStyle->SetTimeOffset(0);
//    mg->GetXaxis()->SetTimeDisplay(1);
//    mg->GetXaxis()->SetTimeFormat("%d-%m-%y");
//    mg->GetXaxis()->SetLabelSize(0.02);
//    mg->GetXaxis()->SetTitle("Date");
//  } else mg->GetHistogram()->GetXaxis()->SetTitle(generateLabel(getX,normalizeToAreaX).c_str());
//
//  if (funcCmp(getY, &RunObject::getSOR) || funcCmp(getY, &RunObject::getEOR)){
//    //This time offset is NEEDED to correctly display data from timestamp!
//    gStyle->SetTimeOffset(0);
//    mg->GetYaxis()->SetTimeDisplay(1);
//    mg->GetYaxis()->SetTimeFormat("%d-%m-%y");
//    mg->GetYaxis()->SetLabelSize(0.02);
//    mg->GetYaxis()->SetTitle("Date");
//  } else mg->GetHistogram()->GetYaxis()->SetTitle(generateLabel(getY,normalizeToAreaY).c_str());

  return mg;
}

template<typename YType, typename CondType>
TGraph *drawTrend(YType (RunObject::*getY)() const,
                  bool normalizeToArea,
                  bool accumulate=false,
                  bool plotAverage=false,
                  MTRPlanes plane=MTRPlanes::kAll,
                  MTRSides side=MTRSides::kBoth,
                  MTRRPCs RPC=MTRRPCs::kAllRPCs,
                  CondType *conditions=nullptr)
{
  return drawCorrelation(&RunObject::getSOR,
                         getY,
                         false,
                         normalizeToArea,
                         accumulate,
                         plotAverage,
                         plane,
                         side,
                         RPC,
                         conditions);
}

template<typename YType, typename CondType>
TMultiGraph *drawTrends(YType (RunObject::*getY)() const,
                        bool normalizeToArea,
                        bool accumulate=false,
                        bool plotAverage=false,
                        MTRPlanes plane=MTRPlanes::kAll,
                        MTRSides side=MTRSides::kBoth,
                        CondType *conditions=nullptr)
{
  return drawCorrelations(&RunObject::getSOR,
                          getY,
                          false,
                          normalizeToArea,
                          accumulate,
                          plotAverage,
                          plane,
                          side,
                          conditions);
}

template<typename YType, typename CondType>
TMultiGraph *
drawMaxMin(YType (RunObject::*getY)() const,
           bool normalizeToAreaY=false,
           bool accumulate=false,
           bool plotAverage=false,
           MTRPlanes plane=MTRPlanes::kAll,
           MTRSides side=MTRSides::kBoth,
           CondType *conditions=nullptr)
{
  auto mg = drawTrends(getY,normalizeToAreaY,accumulate,plotAverage,plane,side,conditions);
  auto grList = mg->GetListOfGraphs();

  auto *mgOut = new TMultiGraph();

  if(plotAverage){
    if( plane<MTRPlanes::kNPlanes ) {
      auto avgGraph = (TGraph *) grList->FindObject(Form("avg_%d", kPlanes[plane]));
      if (avgGraph) mgOut->Add(avgGraph);
    } else {
      auto avgGraph = (TGraph *) grList->FindObject("avg");
      if (avgGraph) mgOut->Add(avgGraph);
    }
  }

  TGraph *minGraph = nullptr;
  TGraph *maxGraph = nullptr;

  double minValue=1e+19;
  double maxValue=0.;

  for(int iGraph = 0; iGraph < grList->GetEntries(); iGraph++){
    auto graph = (TGraph*)(grList->At(iGraph));

    if(strcmp(graph->GetName(),"1_1_6")==0) continue;

    double dummyX;
    double dummyY;

    graph->GetPoint(graph->GetN()-1, dummyX, dummyY);

    if (iGraph==0) {
      minValue = dummyY;
      minGraph = graph;

      maxValue = dummyY;
      maxGraph = graph;
    } else {
      if ( dummyY < minValue ){
        minValue = dummyY;
        minGraph = graph;
      }

      if ( dummyY > maxValue ){
        maxValue = dummyY;
        maxGraph = graph;
      }
    }
  }

  if (minGraph) mgOut->Add(minGraph);
  if (maxGraph) mgOut->Add(maxGraph);

//  mgOut->Draw("ap");
//  mgOut->GetHistogram()->GetXaxis()->SetTimeOffset(0);
//  mgOut->GetHistogram()->GetXaxis()->SetTimeDisplay(1);
//  mgOut->GetHistogram()->GetXaxis()->SetTimeFormat("%d-%m-%y");
//  mgOut->GetHistogram()->GetYaxis()->SetLabelSize(0.02);
//
//  mgOut->GetHistogram()->GetXaxis()->SetTitle("Date");
//  mgOut->GetHistogram()->GetYaxis()->SetTitle(generateLabel(getY,normalizeToAreaY).c_str());

  return mgOut;
}
