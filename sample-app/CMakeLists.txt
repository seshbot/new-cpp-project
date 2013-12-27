cmake_minimum_required (VERSION 2.6)

include(${PROJECT_SOURCE_DIR}/cmake/Config.cmake)

include_directories("${PROJECT_SOURCE_DIR}/sample-lib")

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


add_executable(sample-app sample-app.cpp)

target_link_libraries (sample-app sample)

#
# install
#

install (TARGETS sample-app DESTINATION bin)

