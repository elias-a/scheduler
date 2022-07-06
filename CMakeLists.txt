cmake_minimum_required(VERSION 3.20)
set(CMAKE_CXX_STANDARD 11)

project(scheduler)
add_executable(schedule.o schedule.cpp scheduler.cpp nfl.cpp)

include(FetchContent)
FetchContent_Declare(cpr GIT_REPOSITORY https://github.com/libcpr/cpr.git
                         GIT_TAG ee7f91f5566b6279aadf6d0b1aaf56b11963f527)
FetchContent_MakeAvailable(cpr)

target_link_libraries(schedule.o PRIVATE cpr::cpr)

include(FetchContent)
FetchContent_Declare(tinyxml2 GIT_REPOSITORY https://github.com/leethomason/tinyxml2.git
                         GIT_TAG e45d9d16d430a3f5d3eee9fe40d5e194e1e5e63a)
FetchContent_MakeAvailable(tinyxml2)

target_link_libraries(schedule.o PRIVATE tinyxml2::tinyxml2)
