#===============================================================================
#  CMakeLists.txt (examples)  
# 
#  Copyright (C) 2010-2019 Tobias Toll and Thomas Ullrich
# 
#  This file is part of Sartre.
# 
#  This program is free software: you can redistribute it and/or modify 
#  it under the terms of the GNU General Public License as published by 
#  the Free Software Foundation.   
#  This program is distributed in the hope that it will be useful, 
#  but without any warranty; without even the implied warranty of 
#  merchantability or fitness for a particular purpose. See the 
#  GNU General Public License for more details. 
#  You should have received a copy of the GNU General Public License
#  along with this program.  If not, see <http://www.gnu.org/licenses/>.
#   
#  Author: Thomas Ullrich 
#  Last update: 
#  $Date: 2025-02-26 04:03:44 +0530 (Wed, 26 Feb 2025) $
#  $Author: ullrich $
#===============================================================================
cmake_minimum_required (VERSION 3.30)
project(SARTRE_EXAMPLES)
# cmake_policy(SET CMP0153 OLD)

#
#  Compiler options
#
set(CMAKE_CXX_FLAGS "-W -Wall -Wextra -pedantic -Wno-long-long")
set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_CXX_FLAGS_RELEASE "-O")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

#
#  Set the location of the installed sartre package
#
set(SARTRE_DIR "$ENV{SARTRE_DIR}")
if (IS_DIRECTORY "${SARTRE_DIR}" AND 
    IS_DIRECTORY "${SARTRE_DIR}/lib" AND 
    IS_DIRECTORY "${SARTRE_DIR}/include" AND
    IS_DIRECTORY "${SARTRE_DIR}/gemini" AND
    IS_DIRECTORY "${SARTRE_DIR}/tables")
    message (STATUS "Using Sartre installed in ${SARTRE_DIR}")
else()
    message(FATAL_ERROR "Environment variable SARTRE_DIR does not point to a valid Sartre installation.")
endif()

message(STATUS "System = ${CMAKE_SYSTEM}")

#
#  User can select if Sartre is to be compiled
#  in multithreaded mode or not.
#  Default is off.
#  cmake -DMULTITHREADED=ON .
#
option(MULTITHREADED "Switch on/off multithreaded mode" OFF)
if (MULTITHREADED)
    add_definitions(-DSARTRE_IN_MULTITHREADED_MODE)
    message (STATUS "MULTITHREADED mode is ON")
endif (MULTITHREADED)

#
#  See if optional libraries (Glut & OpenGL) are present.
#  If not skip the examples that require them.
#

# GLUT
#find_package(GLUT)
#if (GLUT_FOUND)
#   message("blah")
#   include_directories(${GLUT_INCLUDE_DIR})
#   set(LIBS ${LIBS} ${GLUT_LIBRARIES})
#endif()

# OpenGL
find_package(OpenGL)
if (OPENGL_FOUND)
   include_directories(${OPENGL_INCLUDE_DIR})
   set(LIBS ${LIBS} ${OPENGL_LIBRARIES})
endif()

#
#  Built all programs in example directory
#
add_executable(createBSatBDependenceTable createBSatBDependenceTable.cpp)
add_executable(heraCompare heraCompare.cpp)
add_executable(sartreMain sartreMain.cpp)
add_executable(sartreTest sartreTest.cpp)
add_executable(tableGeneratorMain tableGeneratorMain.cpp)
add_executable(tableGeneratorUPCMain tableGeneratorUPCMain.cpp)
add_executable(lambdaGeneratorMain lambdaGeneratorMain.cpp)
add_executable(convergenceTest convergenceTest.cpp)
add_executable(sartreInclusiveDiffractionMain sartreInclusiveDiffractionMain.cpp) #EicSmearFormatWriter.cpp)
add_executable(tableGeneratorInclusiveDiffractionMain tableGeneratorInclusiveDiffractionMain.cpp)
add_executable(sf sf.cpp)
add_executable(sfMpi sfMpi.cpp)
# add_executable(sf3 sf3.cpp)
add_executable(xsec XSecContribution.cpp)
# add_executable(tablegen tablegen.cpp)
add_executable(tg tableIndis.cpp)
add_executable(tableGeneratorSurrogate tableGeneratorSurrogate.cpp)
# add_executable(EicSmearFormatWriter EicSmearFormatWriter.cpp)
#if (OPENGL_FOUND AND GLUT_FOUND)
#    add_executable(nucleusAnimation nucleusAnimation.cpp)
#endif()    

#
#  Make sure the files to search for external libraries are available
#
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/modules")

#
#  Compiler options
#
set(CMAKE_C_FLAGS_DEBUG " -g -W")
set(CMAKE_CXX_FLAGS_DEBUG " -g -W -Wall -Wextra -pedantic -Wno-long-long -std=c++11")
set(CMAKE_C_FLAGS_RELEASE " -O -W")
set(CMAKE_CXX_FLAGS_RELEASE " -O -W -Wall -Wextra -pedantic -Wno-long-long -std=c++11")
set(CMAKE_C_FLAGS " -O -W")
set(CMAKE_CXX_FLAGS " -O -std=c++11 -W -Wall -Wextra -pedantic -Wno-long-long")

#
#  Include files locations
#
include_directories("${SARTRE_DIR}/include")

#
#  Find external required packages
#  (see also FindGSL.cmake and FindROOT.cmke in cmake/modules)
#

# PYTHIA8
find_package(Pythia8 REQUIRED)
include_directories(${PYTHIA8_INCLUDE_DIR})
set(LIBS ${LIBS} ${PYTHIA8_LIBRARIES})

# GSL
find_package(GSL REQUIRED)
include_directories(${GSL_INCLUDE_DIR})
set(LIBS ${LIBS} ${GSL_LIBRARIES})

# ROOT
find_package(ROOT REQUIRED)
include_directories(${ROOT_INCLUDE_DIR})
set(LIBS ${LIBS} ${ROOT_LIBRARIES} ${ROOT_EG_LIBRARY} ${ROOT_MathMore_LIBRARY}  ${ROOT_Unuran_LIBRARY} )

#BOOST
if (MULTITHREADED)
   set(Boost_USE_MULTITHREADED ON)
   find_package(Boost 1.39 COMPONENTS thread REQUIRED)
   if(Boost_FOUND)
       include_directories(${Boost_INCLUDE_DIR})
       set(LIBS ${LIBS} ${Boost_LIBRARIES} -lboost_system)
   endif(Boost_FOUND)
endif (MULTITHREADED)

#MPI
find_package(MPI REQUIRED)

# ==============================================================================
# LibTorch Configuration
# ==============================================================================
# set(SARTRE_LIBTORCH_DIR "" CACHE PATH "Path to an unpacked libtorch distribution (containing share/cmake/Torch/)")

# if(SARTRE_LIBTORCH_DIR)
#     list(APPEND CMAKE_PREFIX_PATH "${SARTRE_LIBTORCH_DIR}")
# endif()

# find_package(Torch REQUIRED)

# NOTE: LibTorch and ROOT must be built against compatible C++ ABIs
# (_GLIBCXX_USE_CXX11_ABI). If you see ODR/link errors involving std::string
# or std::vector symbols once this is enabled, that ABI mismatch is usually why.

# Apply PyTorch C++ compiler flags globally or per target
# set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${TORCH_CXX_FLAGS}")

# Ensure macOS rpath is configured so the dynamic linker finds .dylib files at runtime
set(CMAKE_MACOSX_RPATH TRUE)
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)



#
#  Libraries to link with
#
#link_directories(${SARTRE_DIR}/lib)
file (GLOB ALLSARTRE ${SARTRE_DIR}/lib/*)
set(LIBS ${ALLSARTRE} ${ALLSARTRE} ${LIBS} ${ALLSARTRE})

target_link_libraries(createBSatBDependenceTable ${LIBS})
target_link_libraries(heraCompare ${LIBS})
target_link_libraries(sartreMain ${LIBS})
target_link_libraries(sartreTest ${LIBS})
target_link_libraries(tableGeneratorMain ${LIBS})
target_link_libraries(tableGeneratorUPCMain ${LIBS})
target_link_libraries(lambdaGeneratorMain ${LIBS})
target_link_libraries(convergenceTest ${LIBS})
target_link_libraries(sartreInclusiveDiffractionMain ${LIBS} ) #${TORCH_LIBRARIES})
target_link_libraries(tableGeneratorInclusiveDiffractionMain ${LIBS})
target_link_libraries(sf ${LIBS} )#${TORCH_LIBRARIES})
target_link_libraries(sf MPI::MPI_CXX)
target_link_libraries(sfMpi ${LIBS} MPI::MPI_CXX)
# target_link_libraries(sf3 ${LIBS} MPI::MPI_CXX)
target_link_libraries(xsec ${LIBS})
# target_link_libraries(tablegen ${LIBS})
target_link_libraries(tg ${LIBS} )# ${TORCH_LIBRARIES})
# target_link_libraries(tablegen MPI::MPI_CXX)
target_link_libraries(tg MPI::MPI_CXX)
target_link_libraries(tableGeneratorSurrogate ${LIBS} MPI::MPI_CXX)
# target_link_libraries(EicSmearFormatWriter ${LIBS})



#if (OPENGL_FOUND AND GLUT_FOUND)
#    target_link_libraries(nucleusAnimation ${LIBS})
#endif()
