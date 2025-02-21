include_directories(Transport
                   # transport/udp_socket
)
add_executable (Agent
                Transport/UDPSocket/UDPSocket.cpp
                Transport/TFTPClient.cpp
                Agent/main.cpp
)

add_library(udp_socket Transport/UDPSocket/UDPSocket.cpp)
add_library(tftp_client Transport/TFTPClient.cpp)