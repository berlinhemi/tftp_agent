include_directories(${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR})
include_directories(${gmock_SOURCE_DIR}/include ${gmock_SOURCE_DIR})
include_directories(${CMAKE_SOURCE_DIR}/thirdparty/easyloggingpp/src)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${PROJECT_SOURCE_DIR}/src)

# Coverage support
option(ENABLE_COVERAGE "Enable coverage reporting" OFF)

if(ENABLE_COVERAGE)
    set(COVERAGE_COMPILE_FLAGS 
        -fprofile-arcs
        -ftest-coverage
        -O0
        -g
    )
    set(COVERAGE_LINK_FLAGS 
        -fprofile-arcs
        -ftest-coverage
        -lgcov
    )
else()
    set(COVERAGE_COMPILE_FLAGS "")
    set(COVERAGE_LINK_FLAGS "")
endif()

file(GLOB TEST_SRC_FILES ${PROJECT_SOURCE_DIR}/test/*.cpp)

add_executable(TFTPCliTest UnittestTFTPClient.cpp)

add_executable(AgentExecutorTest 
        UnittestExecutor.cpp Base64.cpp)

add_executable(AgentPackerTest 
        UnittestPacker.cpp Base64.cpp)

add_executable(AgentTest
        UnittestAgent.cpp)

        
# if(ENABLE_COVERAGE)
#         target_compile_options(TFTPCliTest PRIVATE ${COVERAGE_COMPILE_FLAGS})
#         target_compile_options(AgentExecutorTest PRIVATE ${COVERAGE_COMPILE_FLAGS})
#         target_compile_options(AgentPackerTest PRIVATE ${COVERAGE_COMPILE_FLAGS})
#         target_compile_options(AgentTest PRIVATE ${COVERAGE_COMPILE_FLAGS})
        
#         target_link_options(TFTPCliTest PRIVATE ${COVERAGE_LINK_FLAGS})
#         target_link_options(AgentExecutorTest PRIVATE ${COVERAGE_LINK_FLAGS})
#         target_link_options(AgentPackerTest PRIVATE ${COVERAGE_LINK_FLAGS})
#         target_link_options(AgentTest PRIVATE ${COVERAGE_LINK_FLAGS})
# endif()

target_link_libraries(TFTPCliTest 
        udp_socket tftp_client 
        gtest gmock)

target_link_libraries(AgentExecutorTest 
        executor 
        gtest gmock)

target_link_libraries(AgentPackerTest 
        packer 
        ZLIB::ZLIB OpenSSL::SSL OpenSSL::Crypto 
        gtest gmock)

target_link_libraries(AgentTest
        agent packer executor tftp_client udp_socket
        ZLIB::ZLIB OpenSSL::SSL OpenSSL::Crypto
        gtest gmock)


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

# ============ ЦЕЛЬ ДЛЯ ГЕНЕРАЦИИ COVERAGE ОТЧЕТА ============
if(ENABLE_COVERAGE)
    find_program(LCOV_EXECUTABLE lcov)
    find_program(GENHTML_EXECUTABLE genhtml)
    
    if(LCOV_EXECUTABLE AND GENHTML_EXECUTABLE)
        add_custom_target(coverage
            COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure
            COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/coverage_report
            # Сканируем ВСЮ папку сборки, а не только текущую
            COMMAND ${LCOV_EXECUTABLE} --capture --directory ${CMAKE_BINARY_DIR} 
                    --output-file ${CMAKE_BINARY_DIR}/coverage.info
                    --gcov-tool /usr/bin/gcov-14
                    --ignore-errors inconsistent,version
            COMMAND ${LCOV_EXECUTABLE} --remove ${CMAKE_BINARY_DIR}/coverage.info 
                    "/usr/*" 
                    "${CMAKE_SOURCE_DIR}/thirdparty/*" 
                    "${CMAKE_SOURCE_DIR}/test/*"
                    "${CMAKE_BINARY_DIR}/CMakeFiles/*"
                    "${CMAKE_BINARY_DIR}/generators/*"
                    --output-file ${CMAKE_BINARY_DIR}/coverage_filtered.info
                    --gcov-tool /usr/bin/gcov-14
                    --ignore-errors unused
            COMMAND ${GENHTML_EXECUTABLE} ${CMAKE_BINARY_DIR}/coverage_filtered.info 
                    --output-directory ${CMAKE_BINARY_DIR}/coverage_report
                    --title "TFTP Agent Coverage Report"
                    --legend
                    --ignore-errors missing
            WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
            DEPENDS TFTPCliTest AgentExecutorTest AgentPackerTest AgentTest
            COMMENT "Generating code coverage report with lcov"
        )
        
        add_custom_target(cov
            COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} --target coverage
            WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
            COMMENT "Alias for coverage target"
        )
        
        message(STATUS "Coverage target added: 'make coverage' or 'make cov'")
    else()
        message(WARNING "lcov or genhtml not found - install: sudo apt-get install lcov")
    endif()
endif()
# ================================================================