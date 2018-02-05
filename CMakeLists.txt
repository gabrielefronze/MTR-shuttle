cmake_minimum_required(VERSION 3.6)
project(MTR-shuttle)

set(CMAKE_CXX_STANDARD 11)

set(SOURCE_FILES
        RunObject.h
        RunObject.cxx
        AMANDAData.h
        AMANDAData.cxx
        AMANDACurrent.h
        AMANDACurrent.cxx
        MTRShuttle.h
        MTRShuttle.cxx
        Parameters.h
        AlienUtils.h
        CompileAll.c
        testMTRShuttle.c
        )

add_executable(alice_MTR_utils ${SOURCE_FILES})

include_directories(/usr/include
        /Users/Gabriele/alice_sw/sw/osx_x86-64/ROOT/v5-34-30-alice8-1/include
        /Users/Gabriele/alice_sw/sw/osx_x86-64/AliRoot/latest/include
        /Users/Gabriele/alice_sw/sw/osx_x86-64/AliPhysics/latest/include
        /usr/local/opt/libiomp/include/libiomp)
