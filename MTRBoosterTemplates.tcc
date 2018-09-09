//
// Created by Gabriele Gaetano Fronzé on 20/03/2018.
//

#include "MTRBooster.h"
#include "type_traits"

template<typename XType, typename YType>
TGraph *drawCorrelation(XType (RunObject::*getX)(MTRPlanes p, MTRSides s, MTRRPCs r) const,
                        YType (RunObject::*getY)(MTRPlanes p, MTRSides s, MTRRPCs r) const,
                        bool normalizeToAreaX=false,
                        bool normalizeToAreaY=false,
                        bool accumulate=false,
                        bool plotAverage=false,
                        MTRPlanes plane=MTRPlanes::kNPlanes,
                        MTRSides side=MTRSides::kNSides,
                        MTRRPCs RPC=MTRRPCs::kNRPCs,
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

  returnedGraph->GetXaxis()->SetLabelSize(0.035);
  returnedGraph->GetYaxis()->SetLabelSize(0.035);
  if(!plotAverage){
    returnedGraph->SetLineColor(kColors[RPC]);
    returnedGraph->SetMarkerColor(kColors[RPC]);
    returnedGraph->SetMarkerStyle(kMarkers[plane]);
    returnedGraph->SetMarkerSize(0.1);
    returnedGraph->SetLineStyle((Style_t)(1));
    returnedGraph->SetLineWidth(1);
  } else {
    returnedGraph->SetLineColor(kBlack);
    returnedGraph->SetMarkerColor(kBlack);
    returnedGraph->SetMarkerStyle(20);
    returnedGraph->SetMarkerSize(0.15);
    returnedGraph->SetLineStyle((Style_t)(1));
    returnedGraph->SetLineWidth(2);
  }

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

  auto yCumulus = (YType)0.;

//  auto dataVector = (!plotAverage)?fShuttle.fRunDataVect[plane][side][RPC]:fShuttle.fRunDataVectAvg[(plane<MTRPlanes::kNPlanes)?plane:4];

  for( auto const &dataIt : fShuttle.fRunDataVect){

    bool shouldPlot = true;
    for (const auto &itCondition : conditions->fConditions) {
      shouldPlot &= itCondition(&dataIt);
    }
   if (!shouldPlot) continue;

    XType x = (dataIt.*getX)(plane,side,RPC);
    YType y = (dataIt.*getY)(plane,side,RPC);

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

    for(auto &itReplaced : this->fReplacedRPCs){
      if(itReplaced.shouldReset(dataIt.getSOR(),plane,side,RPC,accumulate) && !(itReplaced.fAlreadyReset)) {
        yCumulus = (YType)0;
        itReplaced.fAlreadyReset = true;
      }
    }

    returnedGraph->SetPoint(counter++,(double)x,(double)((accumulate)?(yCumulus+=y):y));
  }

  this->resetReplacedRPCs();

  return returnedGraph;
}

template<typename XType, typename YType, class ...Args>
TGraph *drawCorrelation(XType (RunObject::*getX)(MTRPlanes p, MTRSides s, MTRRPCs r) const,
                                    YType (RunObject::*getY)(MTRPlanes p, MTRSides s, MTRRPCs r) const,
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
TMultiGraph *drawCorrelations(XType(RunObject::*getX)(MTRPlanes p, MTRSides s, MTRRPCs r) const,
                              YType(RunObject::*getY)(MTRPlanes p, MTRSides s, MTRRPCs r) const,
                              bool normalizeToAreaX=false,
                              bool normalizeToAreaY=false,
                              bool accumulate=false,
                              bool plotAverage=false,
                              MTRPlanes plane=MTRPlanes::kNPlanes,
                              MTRSides side=MTRSides::kNSides,
                              CondType *conditions=nullptr)
{
  auto *mg = new TMultiGraph();

  for (int iPlane=MTRPlanes::kMT11; iPlane<MTRPlanes::kNPlanes; iPlane++) {
    if ( plane!=MTRPlanes::kNPlanes && iPlane!=plane ) continue;
    for (int iSide =MTRSides::kINSIDE; iSide < MTRSides::kNSides; iSide++) {
      if ( side!=MTRSides::kNSides && iSide!=side ) continue;
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
                      MTRSides::kNSides,
                      kNRPCs,
                      conditions));
  }

  if(!(mg->GetListOfGraphs())) return nullptr;

  return mg;
}

template<typename YType, typename CondType>
TGraph *drawTrend(YType (RunObject::*getY)(MTRPlanes p, MTRSides s, MTRRPCs r) const,
                  bool normalizeToArea,
                  bool accumulate=false,
                  bool plotAverage=false,
                  MTRPlanes plane=MTRPlanes::kNPlanes,
                  MTRSides side=MTRSides::kNSides,
                  MTRRPCs RPC=MTRRPCs::kNRPCs,
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
TMultiGraph *drawTrends(YType (RunObject::*getY)(MTRPlanes p, MTRSides s, MTRRPCs r) const,
                        bool normalizeToArea,
                        bool accumulate=false,
                        bool plotAverage=false,
                        MTRPlanes plane=MTRPlanes::kNPlanes,
                        MTRSides side=MTRSides::kNSides,
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
drawMaxMin(YType (RunObject::*getY)(MTRPlanes p, MTRSides s, MTRRPCs r) const,
           bool normalizeToAreaY=false,
           bool accumulate=false,
           bool plotAverage=false,
           MTRPlanes plane=MTRPlanes::kNPlanes,
           MTRSides side=MTRSides::kNSides,
           CondType *conditions=nullptr)
{
  auto mg = drawTrends(getY,normalizeToAreaY,accumulate,plotAverage,plane,side,conditions);
  auto grList = mg->GetListOfGraphs();

  auto *mgOut = new TMultiGraph();

  TGraph *minGraph = nullptr;
  TGraph *maxGraph = nullptr;

  double minValue=1e+19;
  double maxValue=0.;

  if(accumulate){
    for(int iGraph = 0; iGraph < grList->GetEntries(); iGraph++){
      auto graph = (TGraph*)(grList->At(iGraph));

      bool shouldSkip = false;

      for(auto const& repIt :fReplacedRPCs){
        char replacedRPC[10];
        sprintf(replacedRPC,"%d_%d_%d",repIt.fPlane,repIt.fSide,repIt.fRPC);
        if(strcmp(graph->GetName(),replacedRPC)==0) {
          shouldSkip = true;
          break;
        }
      }

      if(shouldSkip) continue;

      std::string graphName = graph->GetName();

      if (graphName.find("avg")!=std::string::npos){
        if (plotAverage) mgOut->Add(graph);
        continue;
      }

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
  } else {
    for(int iGraph = 0; iGraph < grList->GetEntries(); iGraph++){
      auto graph = (TGraph*)(grList->At(iGraph));
      
      double dummyX;
      double dummyY;
      double cumulusY = 0.;

      bool shouldSkip = false;

      for(auto const& repIt :fReplacedRPCs){
        char replacedRPC[10];
        sprintf(replacedRPC,"%d_%d_%d",repIt.fPlane,repIt.fSide,repIt.fRPC);
        if(strcmp(graph->GetName(),replacedRPC)==0) {
          shouldSkip = true;
          break;
        }
      }

      if(shouldSkip) continue;

      std::string graphName = graph->GetName();

      if (graphName.find("avg")!=std::string::npos){
        if (plotAverage) mgOut->Add(graph);
        continue;
      }

      for(int iPoint = 1; iPoint < graph->GetN(); iPoint++){
        graph->GetPoint(iPoint, dummyX, dummyY);
        cumulusY+=dummyY;
      }

      cumulusY/=graph->GetN()-2;

      if (iGraph==0) {
        minValue = cumulusY;
        minGraph = graph;

        maxValue = cumulusY;
        maxGraph = graph;
      } else {
        if ( cumulusY < minValue ){
          minValue = cumulusY;
          minGraph = graph;
        }

        if ( cumulusY > maxValue ){
          maxValue = cumulusY;
          maxGraph = graph;
        }
      }
    }
  }

  if (minGraph) mgOut->Add(minGraph);
  if (maxGraph) mgOut->Add(maxGraph);

  return mgOut;
}
