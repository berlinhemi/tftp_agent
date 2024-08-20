include_directories(${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR})
include_directories(${gmock_SOURCE_DIR}/include ${gmock_SOURCE_DIR})

message(gtest_SOURCE_DIR="${gtest_SOURCE_DIR}")
message(gmock_SOURCE_DIR="${gmock_SOURCE_DIR}")

include_directories(${PROJECT_SOURCE_DIR}/src/transport)
include_directories(${PROJECT_SOURCE_DIR}/src/transport/udp_socket)

file(GLOB TEST_SRC_FILES ${PROJECT_SOURCE_DIR}/test/*.cpp)

#add_executable(runUnitTests ${TEST_SRC_FILES})
add_executable(tftp_cli_test tfpclient_test.cpp)
#message(TEST_SRC_FILES="${TEST_SRC_FILES}")
target_link_libraries(tftp_cli_test gtest gmock udp_socket tftp_client )
# target_link_libraries(tftp_cli_test gtest gmock udp_socket tftp_client ?gtest_main?)

add_test(NAME TFTPCliTest
        COMMAND tftp_cli_test
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        )