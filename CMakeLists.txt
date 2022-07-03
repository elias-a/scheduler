cmake_minimum_required(VERSION 3.20)
set(CMAKE_CXX_STANDARD 11)

project(scheduler)
add_executable(schedule.o schedule.cpp scheduler.cpp nfl.cpp)

include(FetchContent)
FetchContent_Declare(cpr GIT_REPOSITORY https://github.com/libcpr/cpr.git
                         GIT_TAG ee7f91f5566b6279aadf6d0b1aaf56b11963f527) # The commit hash for 1.8.2. Replace with the latest from: https://github.com/libcpr/cpr/releases
FetchContent_MakeAvailable(cpr)

target_link_libraries(schedule.o PRIVATE cpr::cpr)
