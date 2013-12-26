cmake_minimum_required (VERSION 2.6)

project (SampleCppProject)

set (VERSION_MAJOR 0)
set (VERSION_MINOR 1)
set (VERSION_PATCH 1)

# global configuration helpers
include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/Config.cmake)

configure_file (
   "${PROJECT_SOURCE_DIR}/Config.h.in"
   "${PROJECT_BINARY_DIR}/Config.h"
)

install (FILES "${PROJECT_BINARY_DIR}/Config.h" DESTINATION include)

# Initialize CXXFLAGS.
set(CMAKE_CXX_FLAGS                "-Wall -std=c++11")
set(CMAKE_CXX_FLAGS_DEBUG          "-O0 -g")
set(CMAKE_CXX_FLAGS_MINSIZEREL     "-Os -DNDEBUG")
set(CMAKE_CXX_FLAGS_RELEASE        "-O4 -DNDEBUG")
set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-O2 -g")

# Compiler-specific C++11 activation.
if ("${CMAKE_CXX_COMPILER_ID}" MATCHES "GNU")
   execute_process(
      COMMAND ${CMAKE_CXX_COMPILER} -dumpversion OUTPUT_VARIABLE GCC_VERSION)
   if (NOT (GCC_VERSION VERSION_GREATER 4.7 OR GCC_VERSION VERSION_EQUAL 4.7))
      message(FATAL_ERROR "${PROJECT_NAME} requires g++ 4.7 or greater.")
   endif ()
elseif ("${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang")
   set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")
else ()
   message(FATAL_ERROR "Your C++ compiler does not support C++11.")
endif ()


#
# dependent libraries
#

# boost
set(Boost_USE_STATIC_LIBS ON)
set(Boost_USE_MULTITHREADED ON)
find_package(Threads)
#find_package(Boost 1.53.0 COMPONENTS thread date_time filesystem system log log_setup REQUIRED)
find_package(Boost 1.53.0 COMPONENTS log log_setup thread date_time filesystem system REQUIRED)
if(Boost_FOUND)
   include_directories( ${Boost_INCLUDE_DIRS} )
   link_libraries(${CMAKE_THREAD_LIBS_INIT} ${Boost_LIBRARIES})
else(Boost_FOUND)
   message(FATAL_ERROR "Cannot build without Boost. Please set Boost_DIR.")
endif(Boost_FOUND)

#
# child projects
#

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

