#include "udp_socket.h"

#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>

#include <cstring> // memset



UdpSocket::~UdpSocket()
{
    Abort();
}

bool UdpSocket::Init()
{
    // Creating socket file descriptor
    socket_ = socket(AF_INET, SOCK_DGRAM, 0);
    if (socket_ == -1) {
        return false; // socket creation failed
    }

    // read/write timeout
    struct timeval timeout;
    timeout.tv_sec = 2;
    timeout.tv_usec = 0;

    if (setsockopt(socket_, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(timeout)) < 0) {
        return false;
    }
   
    //No need bind(), because sendto() do it implicity

    return true;
}

// bool UdpSocket::Bind(const char *local_addr, uint16_t local_port)
// {
//     if (local_port == 0) {
//         return false;
//     }

//     if (local_addr != nullptr) {
//         strcpy(address_, local_addr);
//     }
//     port_ = local_port;

//     if (!Init()) {
//         return false;
//     }

//     int broadcast = 1;
//     int result = setsockopt(socket_, SOL_SOCKET, SO_BROADCAST, &broadcast, sizeof(broadcast));
//     if (result != 0) {
//         return false;
//     }
    
//     struct sockaddr_in localAddr;
//     memset(&localAddr, '\0', sizeof(localAddr));

//     // Filling server information
//     localAddr.sin_family = AF_INET; // IPv4
//     //    local_addr.sin_addr.s_addr = local_address != nullptr ? ::inet_addr( local_address ) : INADDR_ANY;
//     localAddr.sin_addr.s_addr = INADDR_ANY;
//     localAddr.sin_port = htons(port_);

//     // Bind the socket with the server address
//     result = bind(socket_, (const struct sockaddr *)&localAddr, sizeof(localAddr));
//     if (result != 0) { // bind failed
//         close(socket_);
//         return false;
//     }

//     return true;
// }

// bool UdpSocket::Bind( uint16_t local_port )
// {
//     return Bind(nullptr, local_port);
// }

void UdpSocket::Abort()
{
    if (socket_ != -1) {
        shutdown(socket_, 2);
        close(socket_);
        socket_ = -1;
    }
}

ssize_t UdpSocket::ReadDatagram(std::vector<BYTE>& data, size_t max_len,  std::string& host, uint16_t* port)
{
    if ( socket_ < 0) {
        return 0;
    }
    
    struct sockaddr_in remote_addr;
    socklen_t remote_addr_len = sizeof(struct sockaddr_in);
    memset(&remote_addr, '\0', sizeof(remote_addr));

    const ssize_t size =  recvfrom(socket_
                                    ,&data[0], max_len
                                    ,MSG_WAITFORONE // blocking operation! Use MSG_DONTWAIT for non blocking
                                    ,(struct sockaddr *)&remote_addr, &remote_addr_len);
    if (size > 0) {

        host = inet_ntoa(remote_addr.sin_addr);
        if (port != nullptr) {
            *port = ntohs(remote_addr.sin_port);
        }
    }

    return size;
}

int64_t UdpSocket::WriteDatagram(const std::vector<BYTE>& data, const std::string& host, uint16_t port)
{
    if (data.empty() || socket_ < 0) {
        return 0;
    }

    struct sockaddr_in remote_addr;
    
    remote_addr.sin_family = AF_INET;
    remote_addr.sin_addr.s_addr = inet_addr(host.c_str());
    remote_addr.sin_port = htons(port);
    memset(remote_addr.sin_zero, '\0', sizeof(remote_addr.sin_zero));

    const ssize_t size = sendto(socket_
                                ,&data[0], data.size()
                                ,MSG_DONTWAIT
                                ,(struct sockaddr*)&remote_addr, sizeof(remote_addr));

    return size;
}

// const char* UdpSocket::LocalAddress() const
// {
//     return address_;
// }

// uint16_t UdpSocket::LocalPort() const
// {
//     return port_;
// }
