# MTRShuttle and MTRBooster
## advanced framework for rocket-fast ALICE MTR performance analysis

This project is aimed at providing  consistent approach to the study of the ALICE Muon Trigger System performance.
A common framework has several advantages over reinventing the wheel at each new implementation:

1. **Bug detection:** a larger user base helps detecting bugs;
2. **Bug fixing:** bugs that have been fixed for one user are automatically fixed for everyone;
3. **Shared development:** new features and functions automatically become available for the whole MTR community;
4. **A well established wheel:** no need to reinvent it every now and then;
5. **Portable and compact data:** the output file has to be light, complete and easily shareable;

Monitoring the MTR performance is an important task to ensure the Resistive Plate Chambers (RPCs) are in good shape for data acquisition. Their noise levels can arise due to both ageing and environmental conditions and a constant monitoring is needed.

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

## Algorithmic and code structure
The framework has been developed keeping in mind the possibility to integrate new data sources and to be flexible enough to cope with ALICE upgrade. A highly template-ised code is necessary to provide enough degrees of freedom to be ready for unpredictable upgrades.

An overview of the classes is presented below.

### `Enumerators` and `Parameters`
`Enumerators` contains the definition of the topology of the MTR system. Three `enum`s have been implemented to describe planes, sides and RPCs. The whole framework uses this convention and avoid hard-coded IDs to limit possible bugs.

`Parameters` contains conversion arrays, useful constants such as threshold or max values and plots settings such as color and marker styles.

### `AMANDAData` and `AMANDACurrent`
DCS readings are characterised by a timestamp. `AMANDAData` is a `struct` that contains the timestamp and appropriate class-like getters, setters and constructor. All additional DCS data types have to inherit from `AMANDAData`.

`AMANDACurrent` is a derived class (from `AMANDAData`) that extends the base class to contain information about total, dark and net current. A `isDarkCurrent` flag is available to keep track of the presence of the beam in the LHC during the current reading. This class is also intended to be a sample implementation to add other DCS measurement types to the framework. 

### `RunObject`
`RunObject` contains a set of measurements performed during a run, plus a set of information retrieved from the OCDB, such as run number, SOR or EOR. Each data member is intended to be the averaged value over time for the given parameter during the given run.

A getter has to be implemented for any additional data member, in order to make it plottable.

The class implements several `bool`-returning methods which are used at plotting time to filter the dataset.

### `MTRConditions`
This class is a wrapper of a vector of `bool`-returning methods belonging to `RunObject`. This has been done to allow the application of several filters to the data sample at plotting time. 

Technically it uses the `std::bind` method to push in the vector the condition methods with embedded eventual parameters, passed as a template parameter pack.

### `AlienUtilis`
This suite of utilities contains useful methods to interface with Alien in order to retrieve data from the GRID.

### `MTRShuttle`
`MTRShuttle` is the class that implements the three crucial aspects of the framework:

1. Retrieving and merging of OCDB and DCS data;
2. Processing of data and stacking of new data on top of previously processed information;
3. Representation of data.

It contains, as data members, several `stl` containers to handle 

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
* `drawMaxMin` is called similarly to `drawTrends`, but provides a trend graph for a whole plane or side with only the maximum and minimum behaving RPCs are shown. It requires as an argument a reference to one of the `RunObject` getters as Y values. X values are the runs EOR timestamps. Several options allow to superimpose the average trend, to accumulate the Y value or to normalise to the RPC area X and Y values separately;

## Acknowledgements
This project has been rewamped several times, growing and extending its capabilities at each iteration.

I would like to thank Diego and Martino for introducing me to the retrieval and visualisation of MTR performances, and for their deep and wise checks of both code and produced results.

I would like to thank Max for the countless cross-checks and discussions.

I would like to thank Filippo for pushing the whole project to a new level, introducing new algorithms and advanced C++ coding to improve usability and maintainability of the whole framework.

I would like to thank Laurent for the countless coding suggestions and tips that really made the project to a new level of abstraction and end-user usability.