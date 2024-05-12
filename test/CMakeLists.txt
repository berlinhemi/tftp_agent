include_directories(${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR})
include_directories(${gmock_SOURCE_DIR}/include ${gmock_SOURCE_DIR})

message(gtest_SOURCE_DIR="${gtest_SOURCE_DIR}")
message(gmock_SOURCE_DIR="${gmock_SOURCE_DIR}")

include_directories(${PROJECT_SOURCE_DIR}/src/transport)
include_directories(${PROJECT_SOURCE_DIR}/src/transport/udp_socket)

file(GLOB TEST_SRC_FILES ${PROJECT_SOURCE_DIR}/test/*.cpp)

add_executable(runUnitTests ${TEST_SRC_FILES})
target_link_libraries(runUnitTests gtest gmock udp_socket gtest_main)