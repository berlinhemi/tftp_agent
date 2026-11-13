
include_directories(.
                    ${CMAKE_SOURCE_DIR}/thirdparty/easyloggingpp/src
                    ${ZLIB_INCLUDE_DIRS} # If using Conan's zlib
                    ${OPENSSL_INCLUDE_DIR} # If using Conan's OpenSSL
)

add_executable (Agent
                main.cpp
                Agent/Agent.cpp
                Agent/Executor.cpp
                Agent/Packer.cpp
                Transport/UDPSocket/UDPSocket.cpp
                Transport/TFTPClient.cpp
)

add_executable (PackerUtil
                Tools/packer_main.cpp      
                Agent/Packer.cpp

)

add_executable(UnpackerUtil
    Tools/unpack_main.cpp
    Agent/Packer.cpp
)


target_link_libraries(Agent PRIVATE 
                    ZLIB::ZLIB
                    OpenSSL::SSL 
                    OpenSSL::Crypto
                    ) 

target_link_libraries(PackerUtil PRIVATE 
                    ZLIB::ZLIB
                    OpenSSL::SSL 
                    OpenSSL::Crypto
                    ) 

target_link_libraries(UnpackerUtil PRIVATE
                    ZLIB::ZLIB
                    OpenSSL::SSL
                    OpenSSL::Crypto
)


add_library(udp_socket Transport/UDPSocket/UDPSocket.cpp)
add_library(tftp_client Transport/TFTPClient.cpp)
add_library(executor Agent/Executor.cpp)
add_library(packer Agent/Packer.cpp)
add_library(agent Agent/Agent.cpp)
