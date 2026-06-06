#include "UDPSocket.h"

#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>

#include <cstring> // memset


UdpSocket::UdpSocket()
{
    m_initialized = Init();
}

bool UdpSocket::IsInitialized()
{
    return m_initialized;
}

bool UdpSocket::Init()
{
    // Creating socket file descriptor
    m_socket = socket(AF_INET, SOCK_DGRAM, 0);
    if (m_socket == -1) {
        return false; // socket creation failed
    }

    // read/write timeout
    struct timeval timeout;
    timeout.tv_sec = 3;
    timeout.tv_usec = 0;

    if (setsockopt(m_socket, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(timeout)) < 0) {
        return false;
    }
   
    return true;
}


void UdpSocket::Abort()
{
    if (m_socket != -1) {
        shutdown(m_socket, 2);
        close(m_socket);
        m_socket = -1;
    }
}

ssize_t UdpSocket::ReadDatagram(std::vector<BYTE>& buffer, 
                                size_t max_len, 
                                std::string& host, 
                                uint16_t* port)
{
    if ( max_len < 1  ){
        return -1;
    }
    // Set buffer size if it too small
    if(buffer.size() < max_len){
        buffer.resize(max_len);
    }
    
    struct sockaddr_in remote_addr;
    socklen_t remote_addr_len = sizeof(struct sockaddr_in);
    memset(&remote_addr, '\0', sizeof(remote_addr));

    const ssize_t size =  recvfrom(m_socket,
                                &buffer[0],
                                max_len,
                                MSG_WAITFORONE, // blocking operation! Use MSG_DONTWAIT for non blocking
                                (struct sockaddr *)&remote_addr,
                                &remote_addr_len);
    if (size > 0) {

        host = inet_ntoa(remote_addr.sin_addr);
        if (port != nullptr) {
            *port = ntohs(remote_addr.sin_port);
        }
        // Rezize to recieved dgram size
        buffer.resize(size);
    }

    return size;
}


ssize_t UdpSocket::WriteDatagram(const std::vector<BYTE>& data,
                                const std::string& host, 
                                uint16_t port)
{
    if (data.empty() || m_socket < 0) {
        return 0;
    }

    struct sockaddr_in remote_addr;
    
    remote_addr.sin_family = AF_INET;
    remote_addr.sin_addr.s_addr = inet_addr(host.c_str());
    remote_addr.sin_port = htons(port);
    memset(remote_addr.sin_zero, '\0', sizeof(remote_addr.sin_zero));

    const ssize_t size = sendto(m_socket
                                ,&data[0],
                                 data.size()
                                ,MSG_DONTWAIT
                                ,(struct sockaddr*)&remote_addr, sizeof(remote_addr));

    return size;
}


UdpSocket::~UdpSocket()
{
    Abort();
}