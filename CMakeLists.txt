cmake_minimum_required (VERSION 3.23.0)

project (tftp_agent)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_BUILD_TYPE Debug)

find_package(ZLIB REQUIRED)
find_package(OpenSSL REQUIRED)

# ============ ОПЦИИ ДЛЯ COVERAGE ============
option(ENABLE_COVERAGE "Enable coverage reporting" OFF)

if(ENABLE_COVERAGE)
    # Добавляем флаги ГЛОБАЛЬНО для всех целей
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fprofile-arcs -ftest-coverage -O0 -g")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fprofile-arcs -ftest-coverage -lgcov")
    message(STATUS "Coverage reporting: ENABLED")
else()
    message(STATUS "Coverage reporting: DISABLED")
endif()
# ============================================

enable_testing()
add_subdirectory(src)
add_subdirectory(test)
# Submodules
add_subdirectory(thirdparty/easyloggingpp)
add_subdirectory(thirdparty/googletest)
