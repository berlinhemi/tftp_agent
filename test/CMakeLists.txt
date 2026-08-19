include_directories(${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR})
include_directories(${gmock_SOURCE_DIR}/include ${gmock_SOURCE_DIR})
include_directories(${CMAKE_SOURCE_DIR}/thirdparty/easyloggingpp/src)

#message(gtest_SOURCE_DIR="${gtest_SOURCE_DIR}")
#message(gmock_SOURCE_DIR="${gmock_SOURCE_DIR}")


#include_directories()
include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${PROJECT_SOURCE_DIR}/src)

#include_directories(${PROJECT_SOURCE_DIR}/src/Transport)
#include_directories(${PROJECT_SOURCE_DIR}/src/transport/udp_socket)

file(GLOB TEST_SRC_FILES ${PROJECT_SOURCE_DIR}/test/*.cpp)

#add_executable(runUnitTests ${TEST_SRC_FILES})
add_executable(TFTPCliTest UnittestTFTPClient.cpp)

add_executable(AgentExecutorTest 
        UnittestExecutor.cpp
        Base64.cpp
        )

add_executable(AgentPackerTest 
        UnittestPacker.cpp
        Base64.cpp
        )

add_executable(AgentTest
        UnittestAgent.cpp
        )
#message(TEST_SRC_FILES="${TEST_SRC_FILES}")
#target_link_libraries(TFTPCliTest udp_socket tftp_client gtest gmock easylogging)
target_link_libraries(TFTPCliTest 
                        udp_socket
                        tftp_client
                        gtest
                        gmock)

target_link_libraries(AgentExecutorTest 
                      executor
                      gtest
                      gmock)

target_link_libraries(AgentPackerTest 
                      packer
                      ZLIB::ZLIB
                        OpenSSL::SSL 
                        OpenSSL::Crypto
                      gtest
                      gmock)

target_link_libraries(AgentTest
                      agent
                      packer
                      executor
                      tftp_client
                      udp_socket
                      ZLIB::ZLIB
                        OpenSSL::SSL 
                        OpenSSL::Crypto
                      gtest
                      gmock)
        #easyloggingpp::easyloggingpp)
# target_link_libraries(tftp_cli_test gtest gmock udp_socket tftp_client ?gtest_main?)

add_test(NAME TFTPCliTest
        COMMAND TFTPCliTest
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        )

add_test(NAME AgentExecutorTest
        COMMAND AgentExecutorTest
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        )

add_test(NAME AgentPackerTest
        COMMAND AgentPackerTest
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        )

add_test(NAME AgentTest
        COMMAND AgentTest
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        )