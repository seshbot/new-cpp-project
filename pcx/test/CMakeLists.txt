cmake_minimum_required (VERSION 2.6)
project(pcx-test)

include(${CMAKE_SOURCE_DIR}/cmake/ConfigTest.cmake)

set(SRCROOT ${PROJECT_SOURCE_DIR})
set(HDRROOT ${CMAKE_SOURCE_DIR}/include/${PROJECT_NAME})

#
# build
#

include_directories("${CMAKE_SOURCE_DIR}/include")

set(TESTSOURCES
    TestMain.cpp
    TestMessageBus.cpp
    TestServiceRegistry.cpp
   )

add_executable(test-pcx ${TESTSOURCES})
target_link_libraries(test-pcx pcx)
add_test(pcx-test test-pcx)
