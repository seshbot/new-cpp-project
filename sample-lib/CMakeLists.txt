cmake_minimum_required (VERSION 2.6)

include(${PROJECT_SOURCE_DIR}/cmake/Config.cmake)

add_library(sample sample-lib.cpp)

#
# install
#

install (TARGETS sample DESTINATION bin)
install (
   FILES sample-lib.h
   DESTINATION include
)

