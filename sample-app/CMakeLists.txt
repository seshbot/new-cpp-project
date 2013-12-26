cmake_minimum_required (VERSION 2.6)

include(${PROJECT_SOURCE_DIR}/cmake/Config.cmake)

include_directories("${PROJECT_SOURCE_DIR}/sample-lib")

add_executable(sample-app sample-app.cpp)

target_link_libraries (sample-app sample)

#
# install
#

install (TARGETS sample-app DESTINATION bin)

