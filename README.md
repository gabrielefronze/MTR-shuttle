# MTRShuttle and MTRBooster
#### Advanced framework for rocket-fast ALICE MTR performance analysis

### Introduction
This project is aimed at providing  consistent approach to the study of the ALICE Muon Trigger System performance.
A common framework has several advantages over reinventing the wheel at each new implementation:

1. **Bug detection:** a larger user base helps detecting bugs;
2. **Bug fixing:** bugs that have been fixed for one user are automatically fixed for everyone;
3. **Shared development:** new features and functions automatically become available for the whole MTR community;
4. **A well established wheel:** no need to reinvent it every now and then;
5. **Portable and compact data:** the output file has to be light, complete and easily shareable;

Monitoring the MTR performance is an important task to ensure the Resistive Plate Chambers (RPCs) are in good shape for data acquisition. Their noise levels can arise due to both ageing and environmental conditions and a constant monitoring is needed.

### Table of contents
  * [Data sources](#data-sources)
  * [Fantastic Observables and How to Plot Them](#fantastic-observables-and-how-to-plot-them)
  * [MTRShuttle plotting in brief (but not so)](#mtrshuttle-plotting-in-brief--but-not-so-)
  * [MTRBooster plotting in brief](#mtrbooster-plotting-in-brief)
  * [Classes and code taxonomy](#classes-and-code-taxonomy)
    + [Enumerators and Parameters](#enumerators-and-parameters)
    + [AMANDAData and AMANDACurrent](#amandadata-and-amandacurrent)
    + [RunObject](#runobject)
    + [MTRConditions](#mtrconditions)
    + [AlienUtilis](#alienutilis)
    + [MTRShuttle](#mtrshuttle)
    + [MTRBooster](#mtrbooster)
- [Acknowledgements](#acknowledgements)

### Data sources
Two data sources can be used for the retrieval of the measurements and data: the Online Run Condition Database (OCDB) and the DCS database system (formerly AMANDA, soon DARMA). 

The OCDB provides beam status information, run information (run number, Start of Run [SOR] and End of Run [EOR], run type...), a redundant copy of DCS readings and a cadenced dump of raw scalers readings. OCDB objects are generated and stored only during data acquisition. 

The DCS infrastructure is connected to a pletora of subsystems and sensors whose readings are polled during the whole activity of the ALICE experiment, even when there is no data acquisition ongoing. The DCS data present in the OCDB is a subsample of the readings stored in the DCS database, but no information on run and beam status can be found in the DCS database.

These two data sources have complementary sets of information that can be combined to get the best coverage possible of the MTR running performances.

### Fantastic Observables and How to Plot Them
Several observables are of crucial importance for the diagnosis of the status of the RPCs. Some of them are:

* _Total current:_ the total current provided by the power supply to the detector;
* _Dark current:_ the current provided by the power supply without the beam presence in LHC;
* _Net current:_ the current strictly related to the particle flow through the detector, namely the difference between total current and dark current;
* _Integrated charge:_ the integral over time of the net current. This represents the charge deposed in the detector due to ionising particles;
* _Rate:_ the number of particles hits per second. Net current should be correlated to this parameter. The correlation can give a charge/hit measurement;
* _HV:_ voltage provided by the power supply;

These variables can be represented in multiple ways. Trending them over time can highlight a worsening of a RPC condition, while plotting the variables in a 2D-plot can show correlations between the parameters. An important addition is the possibility to plot the worst and the best RPC in the same plot.

These three plotting styles will be called "trend", "correlation" and "minmax".

Another important aspect is the possibility to plot the average behaviour of a given parameter alongside that of a specific RPC.

### MTRShuttle plotting in brief (but not so)
First step is creating a `MTRShuttle` object and load the wanted data `csv` file, then compute the average values if needed:

```cpp
  MTRShuttle shuttle;

  shuttle.loadData("path/to/data.csv");
  shuttle.computeAverage(); //needed if the average data is required later
```

If needed, a set of conditions (in the form of a `MTRConditions` object) should to be filled with the wanted conditions:

```cpp
  auto TS2017 = (uint64_t)1483291280;

  MTRConditions conditions;
  conditions.addCondition(&RunObject::isAfter,false,TS2017); //To select data points from 1/1/2017
  conditions.addCondition(&RunObject::isValidForIDark,false); //To select data from dark runs with provided HV
```

At this point one may want to generate a trend for a single RPC's dark current:

```cpp
TGraph* trend_MT11_IN_3_iDark = sciattol.drawTrend(&RunObject::getAvgIDark,false,false,false,kMT11,kINSIDE,k3,conditions);
```

Or HV for a group of RPC plotting without conditions:

```cpp
TMultiGraph* trend_MT11_IN_HV = sciattol.drawTrends(&RunObject::getAvgHV,false,false,false,kMT11,kINSIDE);
TMultiGraph* trend_MT11_HV = sciattol.drawTrends(&RunObject::getAvgHV,false,false,false,kMT11);
```

Or plot only the RPCs reaching the maximum and minimum net currents values at the end of the period:

```cpp
TMultiGraph* maxmin_MT11_IN_iNet = sciattol.drawMaxMin(&RunObject::getAvgINet,false,false,false,kMT11,kINSIDE,conditions);
TMultiGraph* maxmin_MT11_iNet = sciattol.drawMaxMin(&RunObject::getAvgINet,false,false,false,kMT11,kBoth,conditions);
```

Same goes for correlations, even if `MTRShuttle::drawMaxMin` cannot apply to correlations:

```cpp
TMultiGraph* corr_MT11_IN_3_iDark_HV = sciattol.drawTrend(&RunObject::getAvgIDark,&RunObject::getAvgHV,false,false,false,false,kMT11,kINSIDE,k3,conditions);
TMultiGraph* corr_MT11_IN_HV_rateBend = sciattol.drawTrends(&RunObject::getAvgHV,&RunObject::getRateBend,false,false,false,false,kMT11,kINSIDE,kAllRPCs,conditions);
```

The whole set of examples might be modified to normalise to the RPC area the values:

```cpp
TGraph* trend_MT11_IN_3_iDark = sciattol.drawTrend(&RunObject::getAvgIDark,false,false,false,kMT11,kINSIDE,k3,conditions);
TGraph* trend_MT11_IN_3_iDark_norm = sciattol.drawTrend(&RunObject::getAvgIDark,true,false,false,kMT11,kINSIDE,k3,conditions);
```

Or to integrate over X:

```cpp
TGraph* trend_MT11_IN_3_intCharge = sciattol.drawTrend(&RunObject::getIntCharge,false,false,false,kMT11,kINSIDE,k3,conditions);
TGraph* trend_MT11_IN_3_intCharge_integrated = sciattol.drawTrend(&RunObject::getAvgIDark,false,true,false,kMT11,kINSIDE,k3,conditions);
```

Or to superimpose the average trend:

```cpp
TMultiGraph* trend_MT11_IN_HV = sciattol.drawTrends(&RunObject::getAvgHV,false,false,false,kMT11,kINSIDE,kAllRPCs,conditions);
TMultiGraph* trend_MT11_IN_HV_avg = sciattol.drawTrends(&RunObject::getAvgHV,false,false,true,kMT11,kINSIDE,kAllRPCs,conditions);
```

### MTRBooster plotting in brief

First step is to create a `MTRBooster` object:

```cpp
MTRBooster booster("path/to/data.csv");
```

The conditions will be passed at each setup ina  human readable form.

At this point one may want to generate a trend for a single RPC's dark current:

```cpp
//This will generate plot with index 0
booster.SetX("Time").SetY("iDark").SetPlane(MTRPlanes::kMT11).SetSide(MTRSides::kINSIDE).SetRPC(MTRRPCs::k3).SetMinDate("01/01/17");
```

Or HV for a group of RPC:

```cpp
//This will generate plot with index 1
booster.SetX("Time").SetY("HV").SetPlane(MTRPlanes::kMT11).SetSide(MTRSides::kINSIDE).SetMinDate("01/01/17");

//This will generate plot with index 2
booster.SetX("Time").SetY("HV").SetPlane(MTRPlanes::kMT11).SetMinDate("01/01/17");
```

Or plot only the RPCs reaching the maximum and minimum net current values at the end of the period:

```cpp
//This will generate plot with index 3
booster.SetX("Time").SetY("iNet").SetPlane(MTRPlanes::kMT11).SetSide(MTRSides::kINSIDE).SetMinDate("01/01/17");

//This will generate plot with index 4
booster.SetX("Time").SetY("iNet").SetPlane(MTRPlanes::kMT11).SetMinDate("01/01/17");
```

Same goes for correlations, even if `MTRShuttle::drawMaxMin` cannot apply to correlations:

```cpp
//This will generate plot with index 5
booster.SetX("iDark").SetY("HV").SetPlane(MTRPlanes::kMT11).SetSide(MTRSides::kINSIDE).SetRPC(MTRRPCs::k3).SetMinDate("01/01/17");

//This will generate plot with index 6
booster.SetX("HV").SetY("rateBend").SetPlane(MTRPlanes::kMT11).SetSide(MTRSides::kINSIDE).SetRPC(MTRRPCs::k3).SetMinDate("01/01/17");
```

The whole set of examples might be modified to normalise to the RPC area the values:

```cpp
//This will generate plot with index 8, same as 0 but normalised
booster.SetX("Time").SetY("iDark").SetPlane(MTRPlanes::kMT11).SetSide(MTRSides::kINSIDE).SetRPC(MTRRPCs::k3).NormalizeToArea().SetMinDate("01/01/17");
```

Or to integrate over X:

```cpp
//This will generate plot with index 9
booster.SetX("Time").SetY("IntCharge").SetPlane(MTRPlanes::kMT11).SetSide(MTRSides::kINSIDE).SetRPC(MTRRPCs::k3).AccumulateY().SetMinDate("01/01/17");
```

Or to superimpose the average trend:

```cpp
//This will generate plot with index 10
booster.SetX("Time").SetY("HV").SetPlane(MTRPlanes::kMT11).SetSide(MTRSides::kINSIDE).PlotAverage().SetMinDate("01/01/17");
```

Once everything has bee setup just call:

```cpp
booster.Launch();
```

And, if one wants to get the n-th `TMultiGraph`, e.g the 0-th:

```cpp
auto trend_MT11_IN_3_iDark = booster.GetPlot(0);
```

In addition one can plot it directly:

```cpp
auto canv = new TCanvas("canv1","canv1");
booster.AutoDraw(0, canv); //without legend
booster.AutoDraw(0, canv, true); //with automatic legend generation
```

### Classes and code taxonomy
The framework has been developed keeping in mind the possibility to integrate new data sources and to be flexible enough to cope with ALICE upgrade. A highly template-ised code is necessary to provide enough degrees of freedom to be ready for unpredictable upgrades.

An overview of the classes is presented below.

#### Enumerators and Parameters
`Enumerators` contains the definition of the topology of the MTR system. Three `enum`s have been implemented to describe planes, sides and RPCs. The whole framework uses this convention and avoid hard-coded IDs to limit possible bugs.

`Parameters` contains conversion arrays, useful constants such as threshold or max values and plots settings such as color and marker styles.

#### AMANDAData and AMANDACurrent
DCS readings are characterised by a timestamp. `AMANDAData` is a `struct` that contains the timestamp and appropriate class-like getters, setters and constructor. All additional DCS data types have to inherit from `AMANDAData`.

`AMANDACurrent` is a derived class (from `AMANDAData`) that extends the base class to contain information about total, dark and net current. A `isDarkCurrent` flag is available to keep track of the presence of the beam in the LHC during the current reading. This class is also intended to be a sample implementation to add other DCS measurement types to the framework. 

#### RunObject
`RunObject` contains a set of measurements performed during a run, plus a set of information retrieved from the OCDB, such as run number, SOR or EOR. Each data member is intended to be the averaged value over time for the given parameter during the given run.

A getter has to be implemented for any additional data member, in order to make it plottable.

The class implements several `bool`-returning methods which are used at plotting time to filter the dataset.

#### MTRConditions
This class is a wrapper of a vector of `bool`-returning methods belonging to `RunObject`. This has been done to allow the application of several filters to the data sample at plotting time. 

Technically it uses the `std::bind` method to push in the vector the condition methods with embedded eventual parameters, passed as a template parameter pack.

#### AlienUtilis
This suite of utilities contains useful methods to interface with Alien in order to retrieve data from the GRID.

#### MTRShuttle
`MTRShuttle` is the class that implements the three crucial aspects of the framework:

1. Retrieving and merging of OCDB and DCS data;
2. Processing of data and stacking of new data on top of previously processed information;
3. Representation of data.

**Data retrieving**

Several methods constitute the procedure to retrieve data from OCDB and DCS.

* `parseRunList` parses a provided text file containing the run numbers of the runs one wants to obtain from OCDB;
* `parseOCDB` requires a string as argument which corresponds to the path on which the OCDB can be found. Please not that the path can either be a local one or a remote (Alien) one;
* `parseAMANDAiMon` requires a path to the DCS database dump file. This method parses the file acquiring the `iMon` readings. Additional methods have to be implemented to parse DCS measurements described using other aliases;

**Data processing**

Once data has been obtained parsing OCDB and DCS files it can be processed using the following methods.

* `propagateAMANDA` propagates DCS information to the `RunObject`s created while parsing the OCDB. The DCS has no knowing of the ongoing run, hence the propagation of the DCS information has to be performed using the timestamps of DCS reading and SOR and EOR timestamps from OCDB;
* `saveData` dumps the `RunObject` vectors to a `.csv` file with provided path;
* `loadData` loads the `RunObject` vectors from a `.csv` file with provided path;
* `computeAverage` fills several `RunObject` vectors with the average behaviour of the RPCs, grouped by plane. In order to reduce the information redundancy the average vectors are not saved. Calling `computeAverage` is hence necessary after `loadData` if one intends to plot the average behaviours.

**Data representation**

The processed data can now be represented as a "trend", a "correlation" or a "minmax" plot.
Please note that all the plotting methods are specialised calls of a base method called `drawCorrelation`, which contains the drawing algorithm implementation.

* `drawTrend` and `drawTrends` are two methods that allow one to plot a "trend" for a single RPC or for a full plane or side respectively. They require as an argument a reference to one of the `RunObject` getters as Y values. X values are the runs EOR timestamps. Several options allow to superimpose the average trend, to accumulate the Y value or to normalise to the RPC area X and Y values separately;
* `drawCorrelation` and `drawCorrelations` are two methods that allow one to plot a "correlation" for a single RPC or for a full plane or side respectively. They require as first two arguments two references to `RunObject` getters, for X and Y point coordinates respectively. Several options allow to superimpose the average correlation, to accumulate the Y value or to normalise to the RPC area X and Y values separately;
* `drawMaxMin` is called similarly to `drawTrends`, but provides a trend graph for a whole plane or side with only the maximum and minimum behaving RPCs are shown. It requires as an argument a reference to one of the `RunObject` getters as Y values. X values are the runs EOR timestamps. Several options allow to superimpose the average trend, to accumulate the Y value or to normalise to the RPC area X and Y values separately.

#### MTRBooster
`MTRBooster` is an interface class aimed at simplifying the usage of `MTRShuttle` for **data representation**. 
**Data retrieving** and **data processing** functions are only accessible through `MTRShuttle` directly, or using the `MTRBooster::getShuttle` method, as this class owns a `MTRShuttle` data member.

`MTRBooster` strongly relies on a set of methods that can be concatenated in order to specify the settings for the generation of a given plot.
Plot settings are stored in a `MTRPlotSettings` vector.
Several kinds of set-up methods are available:

* **X-Y data selectors**: these methods allow to automatically set the data members to plot in the X and Y axes. One call for each axis has to be made before stacking the stage;
* **Plane, side and RPC selectors**: these methods are intended to select the wanted RPC(s) to plot. One can select only a plane, a side or a combination of a plane and a side (leaving unset the RPC ID), obtaining a `TMultigraph` that contains the overlapped plots for all the RPCs satisfying the selection;
* **Time and run range selectors**: these methods are wrappers of the `RunObject` methods. By calling them one can select time or run number limits. Note that both open and closed intervals can be set (lass than, more than, between);
* **Run type and run conditions selectors**: these methods are meant to select runs which are significant for rather specific plots, e.g. dark runs with provided HV, runs with or without beam presence, etc. They wrap around `RunObject` methods;
* **Plot specific methods**: particular plotting functions are implemented through these methods. They allow to superimpose the average behaviour plot (`MTRBooster::PlotAverage`), to normalise Y (and X for correlation plots) to RPCs areas (`MTRBooster::NormalizeToArea`), integrate over Y (each point is added to the previous and plotted, `MTRBooster::AccumulateY`) or to plot only the trends of the RPCs reaching maximum and minimum values at the end of the selected time windows (`MTRBooster::PlotMinMax`).

All the listed methods interface with a `MTRPlotSettings` object which packs the settings.

After having called the selected chain of methods **is mandatory** to call `MTRBooster::StackStage`, as this call pushes the packed settings in the settings vector and prepares `MTRBooster` to accept a new settings package.
The ordering of the output graphs is the same of the `MTRBooster::StackStage` calls.

Once all the desired stages (aka `MTRPlotSettings`) have been stacked by calling `MTRBooster::StackStage`, the `MTRBooster::Launch` methods allow to generate all the graphs corresponding to the pushed settings. Note that `MTRBooster::Launch` can be called without arguments (generating all the set-up graphs, stored in an internal `vector`) or with a integer argument and a `TMultiGraph` pointer in order to generate the i-th graph and use the provided pointer to reference the generated plot.

The setup of graph style, axis labels, plot name and legend generation are automatically handled by the `MTRBooster::AutoDraw` method. Using this method to draw the graphs allows one to have consistent color coding between several calls. Additionally this method allows to automatically generate a legend of the superimposed plots.

## Acknowledgements
> This project has been revamped several times, growing and extending its capabilities at each iteration.
> 
> I would like to thank Diego and Martino for introducing me to the retrieval and visualisation of MTR performances, and for their deep and wise checks of both code and produced results.
> 
> I would like to thank Max for the countless cross-checks and discussions.
> 
> I would like to thank Laurent for the countless coding suggestions and tips that really made the project to a new level of abstraction and end-user usability.
> 
> I would like to thank Filippo for pushing the whole project to a new level, developing good part of the plotting algorithms and data storage instructions.