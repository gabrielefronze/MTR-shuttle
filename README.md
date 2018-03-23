# MTRShuttle
## advanced framework for rocket-fast ALICE MTR performance analysis

This project is aimed at providing  consistent approach to the study of the ALICE Muon Trigger System performance.
A common framework has several advantages over reinventing the wheel at each new implementation:

1. **Bug detection:** a larger user base helps detecting bugs;
2. **Bug fixing:** bugs that have been fixed for one user are automatically fixed for everyone;
3. **Shared development:** new features and functions automatically become available for the whole MTR community;
4. **A well established wheel:** no need to reinvent it every now and then!

### At first there was a use case scenario
Monitoring the MTR performance is an important task to ensure the Resistive Plate Chambers are in good shape for data acquisition. Their noise levels can arise due to both ageing and environmental conditions and a constant monitoring is needed.

Two data sources can be used for the retrieval of the measurements and data: the Online Run Condition Database (OCDB) and the DCS database system (formerly AMANDA, soon DARMA). 

The OCDB provides beam status information, run information (run number, Start of Run and End of Run, run type...), a redundant copy of DCS readings and a cadenced dump of raw scalers readings. OCDB objects are generated and stored only during data acquisition. 

The DCS infrastructure is connected to a pletora of subsystems and sensors whose readings are polled during the whole activity of the ALICE experiment, even when there is no data acquisition ongoing.


### Acknowledgements
This project has been rewamped several times, growing and extending its capabilities at each iteration.

I would like to thank Diego and Martino for introducing me to the retrieval and visualisation of MTR performances, and for their deep and wise checks of both code and produced results.

I would like to thank Max for the countless cross-checks and discussions.

I would like to thank Filippo for pushing the whole project to a new level, introducing new algorithms and advanced C++ coding to improve usability and maintainability of the whole framework.

I would like to thank Laurent for the countless coding suggestions and tips that really made the project to a new level of abstraction and end-user usability.