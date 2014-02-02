
#
# dependent libraries
#

# boost
if(Boost_FOUND)
   include_directories(SYSTEM ${Boost_INCLUDE_DIRS})
else(Boost_FOUND)
   message(FATAL_ERROR "Cannot build without Boost. Please set Boost_DIR.")
endif(Boost_FOUND)
