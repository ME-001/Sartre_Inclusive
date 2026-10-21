# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION ${CMAKE_VERSION}) # this file comes with cmake

# If CMAKE_DISABLE_SOURCE_CHANGES is set to true and the source directory is an
# existing directory in our source tree, calling file(MAKE_DIRECTORY) on it
# would cause a fatal error, even though it would be a no-op.
if(NOT EXISTS "/Users/bhakta/InDis4/cuba")
  file(MAKE_DIRECTORY "/Users/bhakta/InDis4/cuba")
endif()
file(MAKE_DIRECTORY
  "/Users/bhakta/InDis4/cuba/src/cuba-build"
  "/Users/bhakta/InDis4/cuba"
  "/Users/bhakta/InDis4/cuba/tmp"
  "/Users/bhakta/InDis4/cuba/src/cuba-stamp"
  "/Users/bhakta/InDis4/cuba/src"
  "/Users/bhakta/InDis4/cuba/src/cuba-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "/Users/bhakta/InDis4/cuba/src/cuba-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "/Users/bhakta/InDis4/cuba/src/cuba-stamp${cfgdir}") # cfgdir has leading slash
endif()
