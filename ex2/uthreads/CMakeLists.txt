cmake_minimum_required(VERSION 3.16)
project(uthreads)

set(CMAKE_CXX_STANDARD 14)

add_library(uthreads uthreads.cpp uthreads.h Scheduler.cpp Scheduler.h Thread.cpp Thread.h
        ThreadManager.cpp ThreadManager.h)