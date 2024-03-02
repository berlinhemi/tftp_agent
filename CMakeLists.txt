cmake_minimum_required (VERSION 3.21.0)
project (tftp_agent)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_subdirectory(src)
add_subdirectory(test)

enable_testing()