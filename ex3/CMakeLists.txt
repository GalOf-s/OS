cmake_minimum_required(VERSION 3.21)
project(ex3)

set(CMAKE_CXX_STANDARD 14)

add_executable(ex3 MapReduceFramework.h MapReduceFramework.cpp MapReduceClient.h SampleClient.cpp Barrier.cpp Barrier.h)
