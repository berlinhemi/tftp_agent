cmake_minimum_required (VERSION 3.21.0)
project (tftp_agent)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
#enable_testing()
add_subdirectory(src)
#add_subdirectory(test)
add_subdirectory(googletest)

