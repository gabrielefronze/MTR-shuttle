//
// Created by Gabriele Gaetano Fronzé on 05/02/2018.
//

/*==================================================================*/
/*macro per la compilazione one call di tutte le classi usate       */
#include <TSystem.h>
#include <string>

/*==================================================================*/

void CompileAll(TString myopt="fast"){
std::string opt;
if(myopt.Contains("force")){
opt = "kfg";
}
else {
opt = "kg";
}
gSystem->CompileMacro("AMANDAData.cxx",opt.c_str());
gSystem->CompileMacro("AMANDACurrent.cxx",opt.c_str());
gSystem->CompileMacro("RunObject.cxx",opt.c_str());
gSystem->CompileMacro("MTRShuttle.cxx",opt.c_str());
}
