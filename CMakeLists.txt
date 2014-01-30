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

