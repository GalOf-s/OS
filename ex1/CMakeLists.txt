cmake_minimum_required(VERSION 3.16)
project(osm)

set(CMAKE_CXX_STANDARD 14)

#add_library(osm osm.cpp osm.h)
add_executable(ex1_test test.cpp)

find_library(OSM_LIBRARY osm lib)
target_link_libraries(ex1_test LINK_PUBLIC ${OSM_LIBRARY})

