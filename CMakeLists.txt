cmake_minimum_required(VERSION 3.20)
set(CMAKE_CXX_STANDARD 11)

project(scheduler)
add_executable(schedule.o schedule.cpp scheduler.cpp nfl.cpp)

find_package(PkgConfig REQUIRED)

pkg_check_modules(LIBXMLXX libxml++-2.6)
add_definitions(${LIBXMLXX_CFLAGS})
add_definitions(${LIBXMLXX_LIBS})

pkg_check_modules(LIBCURLPP curlpp)
add_definitions(${LIBCURLPP_CFLAGS})
add_definitions(${LIBCURLPP_LIBS})

TARGET_LINK_LIBRARIES(schedule.o -lcurl -lcurlpp -lxml2)
