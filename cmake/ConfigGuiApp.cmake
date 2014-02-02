include(${CMAKE_SOURCE_DIR}/cmake/ConfigApp.cmake)

#
# dependent libraries
#

#Find any version 2.X of SFML
#See the FindSFML.cmake file for additional details and instructions
#for info on adding SFML 2 to a CMake project see https://github.com/LaurentGomila/SFML/wiki/Tutorial%3A-Build-your-SFML-project-with-CMake
find_package(SFML 2 REQUIRED system window graphics network audio)
if(SFML_FOUND)
  include_directories(${SFML_INCLUDE_DIR})
  set(LOCAL_LINK_LIBRARIES ${LOCAL_LINK_LIBRARIES} ${SFML_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})
else()
  message(FATAL_ERROR "SFML is required to build GUI applications")
endif()
