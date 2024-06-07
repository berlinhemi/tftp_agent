include_directories(transport
                    transport/udp_socket
)
add_executable (agent
                transport/udp_socket/udp_socket.cpp
                transport/tftp_client.cpp
                agent/main.cpp
)

     


add_library(udp_socket transport/udp_socket/udp_socket.cpp)
add_library(tftp_client transport/tftp_client.cpp)