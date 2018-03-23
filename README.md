# MTRShuttle and MTRBooster
## advanced framework for rocket-fast ALICE MTR performance analysis

This project is aimed at providing  consistent approach to the study of the ALICE Muon Trigger System performance.
A common framework has several advantages over reinventing the wheel at each new implementation:

1. **Bug detection:** a larger user base helps detecting bugs;
2. **Bug fixing:** bugs that have been fixed for one user are automatically fixed for everyone;
3. **Shared development:** new features and functions automatically become available for the whole MTR community;
4. **A well established wheel:** no need to reinvent it every now and then!

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

###


### Acknowledgements
This project has been rewamped several times, growing and extending its capabilities at each iteration.

I would like to thank Diego and Martino for introducing me to the retrieval and visualisation of MTR performances, and for their deep and wise checks of both code and produced results.

I would like to thank Max for the countless cross-checks and discussions.

I would like to thank Filippo for pushing the whole project to a new level, introducing new algorithms and advanced C++ coding to improve usability and maintainability of the whole framework.

I would like to thank Laurent for the countless coding suggestions and tips that really made the project to a new level of abstraction and end-user usability.