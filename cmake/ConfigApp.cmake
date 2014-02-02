
#
# dependent libraries
#

# boost
if(Boost_FOUND)
   include_directories(SYSTEM ${Boost_INCLUDE_DIRS})
   link_libraries(${CMAKE_THREAD_LIBS_INIT} )
   set(LOCAL_LINK_LIBRARIES ${LOCAL_LINK_LIBRARIES} ${Boost_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})
else(Boost_FOUND)
   message(FATAL_ERROR "Cannot build without Boost. Please set Boost_DIR.")
endif(Boost_FOUND)
