cmake_minimum_required (VERSION 3.21.0)

#include(${CMAKE_BINARY_DIR}/conan_paths.cmake)

project (tftp_agent)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_BUILD_TYPE Debug)

find_package(ZLIB REQUIRED)
find_package(OpenSSL REQUIRED)
#find_package(easyloggingpp REQUIRED CONFIG)


enable_testing()
add_subdirectory(src)
add_subdirectory(test)
# Submodules
add_subdirectory(thirdparty/easyloggingpp)
add_subdirectory(thirdparty/googletest)


