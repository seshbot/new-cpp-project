cmake_minimum_required (VERSION 2.6)
project(sample-app)

include(${CMAKE_SOURCE_DIR}/cmake/ConfigApp.cmake)

set(SRCROOT ${PROJECT_SOURCE_DIR})
set(HDRROOT ${CMAKE_SOURCE_DIR}/include/${PROJECT_NAME})

#
# build
#

include_directories("${CMAKE_SOURCE_DIR}/include")

set(SOURCES
   ${SRCROOT}/sample-app.cpp
   )

add_executable(sample-app ${SOURCES})

target_link_libraries(sample-app sample)

#
# install
#

install(TARGETS sample-app DESTINATION bin)

