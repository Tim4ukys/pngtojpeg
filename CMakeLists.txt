cmake_minimum_required(VERSION 3.23.0)
project(pngtojpeg VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 23)

find_package(PNG REQUIRED)
include_directories(${PNG_INCLUDE_DIRS})

find_package(JPEG REQUIRED)
include_directories(${JPEG_INCLUDE_DIRS})

add_executable(pngtojpeg main.cpp)
target_link_libraries(pngtojpeg PRIVATE png jpeg)
