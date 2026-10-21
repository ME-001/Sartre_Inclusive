# Install script for directory: /Users/bhakta/InDis4/src

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "/Users/bhakta/InDis4/build")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "release")
  endif()
  message(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
endif()

# Set the component getting installed.
if(NOT CMAKE_INSTALL_COMPONENT)
  if(COMPONENT)
    message(STATUS "Install component: \"${COMPONENT}\"")
    set(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  else()
    set(CMAKE_INSTALL_COMPONENT)
  endif()
endif()

# Is this installation the result of a crosscompile?
if(NOT DEFINED CMAKE_CROSSCOMPILING)
  set(CMAKE_CROSSCOMPILING "FALSE")
endif()

# Set path to fallback-tool for dependency-resolution.
if(NOT DEFINED CMAKE_OBJDUMP)
  set(CMAKE_OBJDUMP "/Library/Developer/CommandLineTools/usr/bin/objdump")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/sartre/lib" TYPE STATIC_LIBRARY FILES "/Users/bhakta/InDis4/src/libsartre.a")
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/sartre/lib/libsartre.a" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/sartre/lib/libsartre.a")
    execute_process(COMMAND "/Library/Developer/CommandLineTools/usr/bin/ranlib" "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/sartre/lib/libsartre.a")
  endif()
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  include("/Users/bhakta/InDis4/src/CMakeFiles/sartre.dir/install-cxx-module-bmi-release.cmake" OPTIONAL)
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/sartre/lib" TYPE FILE FILES "/Users/bhakta/InDis4/cuba/build/lib/libcuba.a")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/sartre/include" TYPE FILE FILES
    "/Users/bhakta/InDis4/src/AlphaStrong.h"
    "/Users/bhakta/InDis4/src/Amplitudes.h"
    "/Users/bhakta/InDis4/src/BreakupProduct.h"
    "/Users/bhakta/InDis4/src/Constants.h"
    "/Users/bhakta/InDis4/src/CrossSection.h"
    "/Users/bhakta/InDis4/src/DglapEvolution.h"
    "/Users/bhakta/InDis4/src/DipoleModel.h"
    "/Users/bhakta/InDis4/src/DipoleModelParameters.h"
    "/Users/bhakta/InDis4/src/EicSmearFormatWriter.h"
    "/Users/bhakta/InDis4/src/Enumerations.h"
    "/Users/bhakta/InDis4/src/Event.h"
    "/Users/bhakta/InDis4/src/EventGeneratorSettings.h"
    "/Users/bhakta/InDis4/src/ExclusiveFinalStateGenerator.h"
    "/Users/bhakta/InDis4/src/FinalStateGenerator.h"
    "/Users/bhakta/InDis4/src/FrangibleNucleus.h"
    "/Users/bhakta/InDis4/src/GridSpline.h"
    "/Users/bhakta/InDis4/src/GridSplineBetaPolynomials.h"
    "/Users/bhakta/InDis4/src/GridSplineInterpolateSums.h"
    "/Users/bhakta/InDis4/src/InclusiveDiffractiveCrossSections.h"
    "/Users/bhakta/InDis4/src/InclusiveDiffractiveCrossSectionsFromModel.h"
    "/Users/bhakta/InDis4/src/InclusiveDiffractiveCrossSectionsFromTables.h"
    "/Users/bhakta/InDis4/src/InclusiveFinalStateGenerator.h"
    "/Users/bhakta/InDis4/src/Integrals.h"
    "/Users/bhakta/InDis4/src/IntegrandWrappers.h"
    "/Users/bhakta/InDis4/src/Kinematics.h"
    "/Users/bhakta/InDis4/src/ModeFinderFunctor.h"
    "/Users/bhakta/InDis4/src/Nucleon.h"
    "/Users/bhakta/InDis4/src/Nucleus.h"
    "/Users/bhakta/InDis4/src/PhotonFlux.h"
    "/Users/bhakta/InDis4/src/Sartre.h"
    "/Users/bhakta/InDis4/src/SartreInclusiveDiffraction.h"
    "/Users/bhakta/InDis4/src/Settings.h"
    "/Users/bhakta/InDis4/src/SurrogateModelCollection.h"
    "/Users/bhakta/InDis4/src/Table.h"
    "/Users/bhakta/InDis4/src/TableCollection.h"
    "/Users/bhakta/InDis4/src/TableGeneratorNucleus.h"
    "/Users/bhakta/InDis4/src/TableGeneratorSettings.h"
    "/Users/bhakta/InDis4/src/TwoBodyVectorMesonDecay.h"
    "/Users/bhakta/InDis4/src/VectorMesonDecayMass.h"
    "/Users/bhakta/InDis4/src/Version.h"
    "/Users/bhakta/InDis4/src/WaveOverlap.h"
    "/Users/bhakta/InDis4/src/linterp.h"
    "/Users/bhakta/InDis4/src/surrogate_model.h"
    )
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/sartre/include" TYPE FILE FILES "/Users/bhakta/InDis4/cuba/build/include/cuba.h")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/sartre/bin" TYPE EXECUTABLE FILES "/Users/bhakta/InDis4/src/tableInspector")
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/sartre/bin/tableInspector" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/sartre/bin/tableInspector")
    if(CMAKE_INSTALL_DO_STRIP)
      execute_process(COMMAND "/Library/Developer/CommandLineTools/usr/bin/strip" -u -r "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/sartre/bin/tableInspector")
    endif()
  endif()
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  include("/Users/bhakta/InDis4/src/CMakeFiles/tableInspector.dir/install-cxx-module-bmi-release.cmake" OPTIONAL)
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/sartre/bin" TYPE EXECUTABLE FILES "/Users/bhakta/InDis4/src/tableMerger")
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/sartre/bin/tableMerger" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/sartre/bin/tableMerger")
    if(CMAKE_INSTALL_DO_STRIP)
      execute_process(COMMAND "/Library/Developer/CommandLineTools/usr/bin/strip" -u -r "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/sartre/bin/tableMerger")
    endif()
  endif()
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  include("/Users/bhakta/InDis4/src/CMakeFiles/tableMerger.dir/install-cxx-module-bmi-release.cmake" OPTIONAL)
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/sartre/bin" TYPE EXECUTABLE FILES "/Users/bhakta/InDis4/src/tableQuery")
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/sartre/bin/tableQuery" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/sartre/bin/tableQuery")
    if(CMAKE_INSTALL_DO_STRIP)
      execute_process(COMMAND "/Library/Developer/CommandLineTools/usr/bin/strip" -u -r "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/sartre/bin/tableQuery")
    endif()
  endif()
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  include("/Users/bhakta/InDis4/src/CMakeFiles/tableQuery.dir/install-cxx-module-bmi-release.cmake" OPTIONAL)
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/sartre/bin" TYPE EXECUTABLE FILES "/Users/bhakta/InDis4/src/tableDumper")
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/sartre/bin/tableDumper" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/sartre/bin/tableDumper")
    if(CMAKE_INSTALL_DO_STRIP)
      execute_process(COMMAND "/Library/Developer/CommandLineTools/usr/bin/strip" -u -r "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/sartre/bin/tableDumper")
    endif()
  endif()
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  include("/Users/bhakta/InDis4/src/CMakeFiles/tableDumper.dir/install-cxx-module-bmi-release.cmake" OPTIONAL)
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/sartre/bin" TYPE EXECUTABLE FILES "/Users/bhakta/InDis4/src/tablePriority")
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/sartre/bin/tablePriority" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/sartre/bin/tablePriority")
    if(CMAKE_INSTALL_DO_STRIP)
      execute_process(COMMAND "/Library/Developer/CommandLineTools/usr/bin/strip" -u -r "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/sartre/bin/tablePriority")
    endif()
  endif()
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  include("/Users/bhakta/InDis4/src/CMakeFiles/tablePriority.dir/install-cxx-module-bmi-release.cmake" OPTIONAL)
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/sartre/bin" TYPE EXECUTABLE FILES "/Users/bhakta/InDis4/src/tableVarianceMaker")
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/sartre/bin/tableVarianceMaker" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/sartre/bin/tableVarianceMaker")
    if(CMAKE_INSTALL_DO_STRIP)
      execute_process(COMMAND "/Library/Developer/CommandLineTools/usr/bin/strip" -u -r "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/sartre/bin/tableVarianceMaker")
    endif()
  endif()
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  include("/Users/bhakta/InDis4/src/CMakeFiles/tableVarianceMaker.dir/install-cxx-module-bmi-release.cmake" OPTIONAL)
endif()

string(REPLACE ";" "\n" CMAKE_INSTALL_MANIFEST_CONTENT
       "${CMAKE_INSTALL_MANIFEST_FILES}")
if(CMAKE_INSTALL_LOCAL_ONLY)
  file(WRITE "/Users/bhakta/InDis4/src/install_local_manifest.txt"
     "${CMAKE_INSTALL_MANIFEST_CONTENT}")
endif()
