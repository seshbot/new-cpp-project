cmake_minimum_required (VERSION 2.6)

set(CMAKE_LEGACY_CYGWIN_WIN32 0) # Remove when CMake >= 2.8.4 is required

project (SampleCppProject)

set (VERSION_MAJOR 0)
set (VERSION_MINOR 1)
set (VERSION_PATCH 1)

# global configuration helpers
include(${CMAKE_SOURCE_DIR}/cmake/ConfigCommon.cmake)

configure_file (
   "${PROJECT_SOURCE_DIR}/Config.h.in"
   "${PROJECT_BINARY_DIR}/Config.h"
)

install (FILES "${PROJECT_BINARY_DIR}/Config.h" DESTINATION include)


#
# common dependent libraries
#

# boost
set(Boost_USE_STATIC_LIBS ON)
set(Boost_USE_MULTITHREADED ON)
find_package(Threads)
find_package(Boost 1.55.0 COMPONENTS log log_setup thread date_time filesystem system unit_test_framework test_exec_monitor REQUIRED)
if(Boost_FOUND)
else(Boost_FOUND)
   message(FATAL_ERROR "Cannot build without Boost. Please set Boost_DIR.")
endif(Boost_FOUND)


#
# child projects
#

include_directories(${CMAKE_SOURCE_DIR}/include)

add_subdirectory (pcx)
add_subdirectory (gui)

add_subdirectory (sample-lib)
add_subdirectory (sample-app)


#
# installer
#

include (InstallRequiredSystemLibraries)
set (CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE.txt")
set (CPACK_PACKAGE_VERSION_MAJOR "${VERSION_MAJOR}")
set (CPACK_PACKAGE_VERSION_MINOR "${VERSION_MINOR}")
include (CPack)

