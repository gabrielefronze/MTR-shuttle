//
// Created by Gabriele Gaetano Fronzé on 01/02/2018.
//

#ifndef MTR_SHUTTLE_ALIENUTILS_H
#define MTR_SHUTTLE_ALIENUTILS_H

#include <cstdio>
#include <cstdlib>
#include <string>

namespace AlienUtils
{
    static bool checkAlienToken() {
      std::string checkTokenBashCommand = R"(if [[ "`alien-token-info | grep "still valid" `" != "" ]]; then echo "1"; else echo "0"; fi)";
      bool returnValue = false;
      FILE * f = popen(checkTokenBashCommand.c_str(), "r");
      char buf[1];
      size_t read = fread((void *)&buf[0], 1, 1, f);
      returnValue = (buf[0] == '1');
      pclose(f);
      return returnValue;
    }

    static void initAlienToken(std::string userName){
      std::string command = "alien-token-init "+userName;
      system(command.c_str());
    }
};

#endif //MTR_SHUTTLE_ALIENUTILS_H
