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
   
    //No need in bind(), because sendto() do it implicity

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
//     int result = setsockopt(m_socket, SOL_SOCKET, SO_BROADCAST, &broadcast, sizeof(broadcast));
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
//     result = bind(m_socket, (const struct sockaddr *)&localAddr, sizeof(localAddr));
//     if (result != 0) { // bind failed
//         close(m_socket);
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
    if (m_socket != -1) {
        shutdown(m_socket, 2);
        close(m_socket);
        m_socket = -1;
    }
}

<<<<<<< HEAD
ssize_t UdpSocket::ReadDatagram(std::vector<BYTE>& data, 
    //BYTE* data, 
    //size_t max_len, 
     std::string& host, uint16_t* port)
{
    if ( m_socket < 0 || data.capacity() < 1 )
    {
    //data.size() < max_len) {
        return 0;
=======
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
>>>>>>> origin/dev
    }
    
    struct sockaddr_in remote_addr;
    socklen_t remote_addr_len = sizeof(struct sockaddr_in);
    memset(&remote_addr, '\0', sizeof(remote_addr));

<<<<<<< HEAD
    const ssize_t size =  recvfrom(m_socket
                                    ,&data[0], data.capacity()
                                    ,MSG_WAITFORONE // blocking operation! Use MSG_DONTWAIT for non blocking
                                    ,(struct sockaddr *)&remote_addr, &remote_addr_len);
=======
    const ssize_t size =  recvfrom(m_socket,
                                &buffer[0],
                                max_len,
                                MSG_WAITFORONE, // blocking operation! Use MSG_DONTWAIT for non blocking
                                (struct sockaddr *)&remote_addr,
                                &remote_addr_len);
>>>>>>> origin/dev
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


<<<<<<< HEAD
int64_t UdpSocket::WriteDatagram(const std::vector<BYTE>& data,/*size_t data_len,*/  const std::string& host, uint16_t port)
=======
int64_t UdpSocket::WriteDatagram(const std::vector<BYTE>& data,
                                const std::string& host, 
                                uint16_t port)
>>>>>>> origin/dev
{
    if (data.empty() || m_socket < 0) {
        return 0;
    }

    // if (data == nullptr || data_len <= 0 || m_socket < 0) 
    // {
    //     return 0;
    // }

    struct sockaddr_in remote_addr;
    
    remote_addr.sin_family = AF_INET;
    remote_addr.sin_addr.s_addr = inet_addr(host.c_str());
    remote_addr.sin_port = htons(port);
    memset(remote_addr.sin_zero, '\0', sizeof(remote_addr.sin_zero));

    const ssize_t size = sendto(m_socket
<<<<<<< HEAD
                                ,&data[0], data.size()
                                 //,data, data_len
=======
                                ,&data[0],
                                 data.size()
>>>>>>> origin/dev
                                ,MSG_DONTWAIT
                                ,(struct sockaddr*)&remote_addr, sizeof(remote_addr));

    return size;
}


UdpSocket::~UdpSocket()
{
    Abort();
}
// const char* UdpSocket::LocalAddress() const
// {
//     return address_;
// }

// uint16_t UdpSocket::LocalPort() const
// {
//     return port_;
// }
