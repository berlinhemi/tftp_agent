include_directories(transport
                    transport/udp_socket
)
add_executable (agent
                transport/udp_socket/udp_socket.cpp
                transport/tftp_client.cpp
                agent/main.cpp
)
