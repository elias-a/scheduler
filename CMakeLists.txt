cmake_minimum_required(VERSION 3.20)
set(CMAKE_CXX_STANDARD 11)

project(scheduler)
add_executable(schedule.o schedule.cpp scheduler.cpp nfl.cpp)

find_package(PkgConfig REQUIRED)

TARGET_LINK_LIBRARIES(schedule.o -lcurl -lxml2)
