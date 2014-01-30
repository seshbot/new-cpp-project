cmake_minimum_required (VERSION 2.6)
project(gui)

include(${CMAKE_SOURCE_DIR}/cmake/Config.cmake)

set(SRCROOT ${PROJECT_SOURCE_DIR})
set(HDRROOT ${CMAKE_SOURCE_DIR}/include/${PROJECT_NAME})

#
# build
#

include_directories("${CMAKE_SOURCE_DIR}/include")

set(SOURCES
   ${SRCROOT}/main.cpp
   )

add_executable(gui ${SOURCES})

target_link_libraries (gui pcx)

#
# install
#

install (TARGETS gui DESTINATION bin)

