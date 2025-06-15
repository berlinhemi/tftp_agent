
include_directories(Transport
                    ${CMAKE_SOURCE_DIR}/thirdparty/easyloggingpp/src
                    ${CMAKE_SOURCE_DIR}/thirdparty/zlib
)

add_executable (Agent
                Agent/main.cpp
                Agent/Packer.cpp
                Transport/UDPSocket/UDPSocket.cpp
                Transport/TFTPClient.cpp
                ${CMAKE_SOURCE_DIR}/thirdparty/easyloggingpp/src/easylogging++.cc
)

target_link_libraries(Agent PRIVATE zlibstatic OpenSSL::SSL OpenSSL::Crypto) 

add_library(easylogging ${CMAKE_SOURCE_DIR}/thirdparty/easyloggingpp/src/easylogging++.cc)
add_library(udp_socket Transport/UDPSocket/UDPSocket.cpp)
add_library(tftp_client Transport/TFTPClient.cpp)