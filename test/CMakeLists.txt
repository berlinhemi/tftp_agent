include_directories(${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR})
include_directories(${gmock_SOURCE_DIR}/include ${gmock_SOURCE_DIR})
include_directories(${CMAKE_SOURCE_DIR}/thirdparty/easyloggingpp/src)

message(gtest_SOURCE_DIR="${gtest_SOURCE_DIR}")
message(gmock_SOURCE_DIR="${gmock_SOURCE_DIR}")


#include_directories()
include_directories(${PROJECT_SOURCE_DIR}/src/Transport)
#include_directories(${PROJECT_SOURCE_DIR}/src/transport/udp_socket)

file(GLOB TEST_SRC_FILES ${PROJECT_SOURCE_DIR}/test/*.cpp)

#add_executable(runUnitTests ${TEST_SRC_FILES})
add_executable(TFTPCliTest UnittestTFTPClient.cpp)
#message(TEST_SRC_FILES="${TEST_SRC_FILES}")
target_link_libraries(TFTPCliTest udp_socket tftp_client gtest gmock easylogging)
# target_link_libraries(tftp_cli_test gtest gmock udp_socket tftp_client ?gtest_main?)

add_test(NAME TFTPCliTest
        COMMAND TFTPCliTest
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        )