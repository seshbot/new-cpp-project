cmake_minimum_required (VERSION 2.6)
project(pcx)

include(${CMAKE_SOURCE_DIR}/cmake/ConfigTest.cmake)

set(SRCROOT ${PROJECT_SOURCE_DIR})
set(HDRROOT ${CMAKE_SOURCE_DIR}/include/${PROJECT_NAME})

#
# build
#

include_directories("${CMAKE_SOURCE_DIR}/include")

set(SOURCES
   ${HDRROOT}/Logging.h
   ${SRCROOT}/Configuration.cpp
   ${HDRROOT}/Configuration.h
   ${SRCROOT}/ModuleRegistry.cpp
   ${HDRROOT}/ModuleRegistry.h
   ${SRCROOT}/ServiceRegistry.cpp
   ${HDRROOT}/ServiceRegistry.h
   ${SRCROOT}/IndexPool.cpp
   ${HDRROOT}/IndexPool.h
   ${HDRROOT}/MessageBus.h
   ${SRCROOT}/Utils.cpp
   ${HDRROOT}/Utils.h
   )

set(IMPL_SOURCES
   ${SRCROOT}/impl/FileConfiguration.h
   ${SRCROOT}/impl/FileConfiguration.cpp
   ${HDRROOT}/impl/BaseDependencyContainer.h
   #${SRCROOT}/impl/BaseDependencyContainer.cpp
   )

add_library(pcx ${SOURCES} ${IMPL_SOURCES})

add_subdirectory(test)
