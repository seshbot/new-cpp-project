cmake_minimum_required (VERSION 2.6)
project(sample-lib)

include(${CMAKE_SOURCE_DIR}/cmake/ConfigLib.cmake)

set(SRCROOT ${PROJECT_SOURCE_DIR})
set(HDRROOT ${CMAKE_SOURCE_DIR}/include/${PROJECT_NAME})

#
# build
#

set(SOURCES
   ${SRCROOT}/sample-lib.cpp
   ${HDRROOT}/sample-lib.h
   )

add_library(sample ${SOURCES})

