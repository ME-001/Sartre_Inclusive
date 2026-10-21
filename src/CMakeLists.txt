#===============================================================================
#  CMakeLists.txt (src)
#
#  Copyright (C) 2010-2025 Tobias Toll and Thomas Ullrich
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
#  $Date: 2025-10-23 23:04:58 +0530 (Thu, 23 Oct 2025) $
#  $Author: ullrich $
#===============================================================================
include(ExternalProject)
cmake_minimum_required (VERSION 3.18)

#
#  Options
#
option(SARTRE_BUILD_TOOLS "Build table CLI tools" ON)

#
#  Policies / defaults
#
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

#
#  Compiler
#
add_compile_options(-Wall -Wextra -Wpedantic)


# Allow in-tree Find*.cmake modules (FindROOT.cmake, FindPythia8.cmake, etc.)
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}")

#
#  Dependencies
#
find_package(ROOT REQUIRED)         # Uses your FindROOT.cmake if needed
find_package(GSL REQUIRED)
find_package(Pythia8 REQUIRED)      # Uses your FindPythia8.cmake

include_directories(${PYTHIA8_INCLUDE_DIR})
set(LIBS ${LIBS} ${PYTHIA8_LIBRARIES})
include_directories(${GSL_INCLUDE_DIR})
include_directories(${ROOT_INCLUDE_DIR})
set(LIBS ${LIBS} ${ROOT_LIBRARIES})

#
#  NOTE: LibTorch is intentionally NOT a dependency of this target anymore.
#  SurrogateCrossSections (surrogateCrossSection.h/.cpp) was rewritten to
#  evaluate the exported surrogate models with a small, dependency-free C++
#  engine (surrogate_engine.h) instead of LibTorch/TorchScript, so `sartre`
#  no longer needs find_package(Torch), TORCH_INCLUDE_DIRS, TORCH_CXX_FLAGS,
#  or TORCH_LIBRARIES, and none of the ROOT/LibTorch ABI-compatibility
#  concerns that used to apply here are relevant any more. If a future
#  change reintroduces a real LibTorch dependency, restore the
#  find_package(Torch REQUIRED) block that used to live here.
#

include_directories(${PROJECT_SOURCE_DIR}/src)
include_directories(${PROJECT_SOURCE_DIR}/gemini)
include_directories(${PROJECT_SOURCE_DIR}/cuba)

#
#  Boost (CMP0167-ready)
#
if(POLICY CMP0167)
  cmake_policy(SET CMP0167 NEW)
endif()
# Prefer Boost's CMake package config first (Boost >= 1.70 built with CMake)
find_package(Boost 1.70 QUIET COMPONENTS thread CONFIG)
# Fallback to legacy FindBoost module on older environments
if(NOT Boost_FOUND)
  if(POLICY CMP0167)
    cmake_policy(SET CMP0167 OLD)
  endif()
  find_package(Boost REQUIRED COMPONENTS thread)
endif()

#
#  Cuba is built using the autoconf shipped with it
#
ExternalProject_Add(
    cuba
    DOWNLOAD_COMMAND ${CMAKE_COMMAND} -E copy_directory ${PROJECT_SOURCE_DIR}/cuba ${PROJECT_BINARY_DIR}/cuba
    SOURCE_DIR ${PROJECT_BINARY_DIR}/cuba
    PREFIX ${PROJECT_BINARY_DIR}/cuba
    CONFIGURE_COMMAND ${PROJECT_BINARY_DIR}/cuba/configure --prefix=${PROJECT_BINARY_DIR}/cuba/build
    BUILD_COMMAND make lib
    BUILD_IN_SOURCE 1
)

#
#  Library target
#
set(SARTRE_SOURCES
   AlphaStrong.cpp
   Amplitudes.cpp
   BreakupProduct.cpp
   CrossSection.cpp
   DglapEvolution.cpp
   DipoleModel.cpp
   DipoleModelParameters.cpp
   EicSmearFormatWriter.cpp
   Event.cpp
   EventGeneratorSettings.cpp
   ExclusiveFinalStateGenerator.cpp
   FinalStateGenerator.cpp
   FrangibleNucleus.cpp
   InclusiveDiffractiveCrossSections.cpp
   InclusiveDiffractiveCrossSectionsFromTables.cpp
   InclusiveDiffractiveCrossSectionsFromModel.cpp    # NEW: reads the trained model directly
   InclusiveFinalStateGenerator.cpp
   Integrals.cpp
   Kinematics.cpp
   ModeFinderFunctor.cpp
   Nucleon.cpp
   Nucleus.cpp
   PhotonFlux.cpp
   Sartre.cpp
   SartreInclusiveDiffraction.cpp
   Settings.cpp
   Table.cpp
   TableCollection.cpp
   TableGeneratorNucleus.cpp
   TableGeneratorSettings.cpp
   TwoBodyVectorMesonDecay.cpp
   VectorMesonDecayMass.cpp
   WaveOverlap.cpp
  SurrogateModelCollection.cpp
  surrogate_model.cpp
)

add_library(sartre STATIC ${SARTRE_SOURCES})
add_dependencies(sartre cuba)

#
#  Table tools (all stand alone programs)
#
add_executable(tableInspector tableInspector.cpp)
add_executable(tableMerger tableMerger.cpp)
add_executable(tableQuery tableQuery.cpp)
add_executable(tableDumper tableDumper.cpp)
add_executable(tablePriority tablePriority.cpp)
add_executable(tableVarianceMaker tableVarianceMaker.cpp)
target_link_libraries(tableInspector sartre ${LIBS})
target_link_libraries(tableMerger sartre ${LIBS})
target_link_libraries(tableQuery sartre ${LIBS})
target_link_libraries(tableDumper sartre ${LIBS})
target_link_libraries(tablePriority sartre ${LIBS})
target_link_libraries(tableVarianceMaker sartre ${LIBS})
add_dependencies(tableInspector sartre)
add_dependencies(tableMerger sartre)
add_dependencies(tableQuery sartre)
add_dependencies(tableDumper sartre)
add_dependencies(tablePriority sartre)
add_dependencies(tableVarianceMaker sartre)

#
#  Install library and include files (make install) within
#  the distribution tree. Top level CMakeLists.txt will install
#  Sartre in final destination.
#
install(TARGETS sartre DESTINATION sartre/lib)
install(FILES "${PROJECT_BINARY_DIR}/cuba/build/lib/libcuba.a" DESTINATION sartre/lib)

FILE(GLOB AllIncludeFiles CONFIGURE_DEPENDS *.h)
install(FILES ${AllIncludeFiles} DESTINATION sartre/include)
install(FILES "${PROJECT_BINARY_DIR}/cuba/build/include/cuba.h" DESTINATION sartre/include)

install(TARGETS tableInspector DESTINATION sartre/bin)
install(TARGETS tableMerger DESTINATION sartre/bin)
install(TARGETS tableQuery DESTINATION sartre/bin)
install(TARGETS tableDumper DESTINATION sartre/bin)
install(TARGETS tablePriority DESTINATION sartre/bin)
install(TARGETS tableVarianceMaker DESTINATION sartre/bin)