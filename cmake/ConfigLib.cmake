
#
# dependent libraries
#

# boost
set(Boost_USE_STATIC_LIBS ON)
set(Boost_USE_MULTITHREADED ON)
find_package(Threads)
find_package(Boost 1.55.0 COMPONENTS log log_setup thread date_time filesystem system REQUIRED)
if(Boost_FOUND)
   include_directories(SYSTEM  ${Boost_INCLUDE_DIRS})
   link_libraries(${CMAKE_THREAD_LIBS_INIT} ${Boost_LIBRARIES})
else(Boost_FOUND)
   message(FATAL_ERROR "Cannot build without Boost. Please set Boost_DIR.")
endif(Boost_FOUND)
