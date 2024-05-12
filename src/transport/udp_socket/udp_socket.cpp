#include "udp_socket.h"

#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>

#include <cstring> // memset

UdpSocket::UdpSocket()
    : port_(0)
    , socket_(-1)
    , address_{}
{
   // memset(address_, 0, 15);
}

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
    /*if (::setsockopt(socket_, SOL_SOCKET, SO_SNDTIMEO, (char *)&timeout, sizeof(timeout)) < 0) {
        return false;
    }*/

    return true;
}

bool UdpSocket::Bind(const char *local_addr, uint16_t local_port)
{
    if (local_port == 0) {
        return false;
    }

    if (local_addr != nullptr) {
        strcpy(address_, local_addr);
    }
    port_ = local_port;

    if (!Init()) {
        return false;
    }

    int broadcast = 1;
    int result = setsockopt(socket_, SOL_SOCKET, SO_BROADCAST, &broadcast, sizeof(broadcast));
    if (result != 0) {
        return false;
    }
    
    struct sockaddr_in localAddr;
    memset(&localAddr, '\0', sizeof(localAddr));

    // Filling server information
    localAddr.sin_family = AF_INET; // IPv4
    //    local_addr.sin_addr.s_addr = local_address != nullptr ? ::inet_addr( local_address ) : INADDR_ANY;
    localAddr.sin_addr.s_addr = INADDR_ANY;
    localAddr.sin_port = htons(port_);

    // Bind the socket with the server address
    result = bind(socket_, (const struct sockaddr *)&localAddr, sizeof(localAddr));
    if (result != 0) { // bind failed
        close(socket_);
        return false;
    }

    return true;
}

bool UdpSocket::Bind( uint16_t local_port )
{
    return Bind(nullptr, local_port);
}

void UdpSocket::Abort()
{
    if (socket_ != -1) {
        shutdown(socket_, 2);
        close(socket_);
        socket_ = -1;
    }
}

ssize_t UdpSocket::ReadDatagram(char *data, size_t maxlen, char *host, uint16_t *port)
{
    if ((data == nullptr) || (maxlen == 0) || (socket_ < 0)) {
        return 0;
    }
    
    struct sockaddr_in remoteAddr;
    socklen_t remoteAddrLen = sizeof(struct sockaddr_in);
    memset(&remoteAddr, '\0', sizeof(remoteAddr));

    const ssize_t size =
            recvfrom(socket_, data, maxlen,
                       MSG_WAITFORONE, // blocking operation! Use MSG_DONTWAIT for non blocking
                       (struct sockaddr *)&remoteAddr, &remoteAddrLen);
    if (size > 0) {
        if (host != nullptr) {
            //inet_pton(AF_INET, inet_ntoa( senderAddr.sin_addr ), host);
            strcpy(host, inet_ntoa(remoteAddr.sin_addr));
        }
        if (port != nullptr) {
            *port = ntohs(remoteAddr.sin_port);
        }
    }

    return size;
}

int64_t UdpSocket::WriteDatagram(const char *data, size_t len, const char *host, uint16_t port)
{
    if ((data == nullptr) || (len == 0) || (socket_ < 0)) {
        return 0;
    }

    struct sockaddr_in remoteAddr;
    
    remoteAddr.sin_family = AF_INET;
    remoteAddr.sin_addr.s_addr = host != nullptr ? inet_addr(host) : INADDR_ANY;
    remoteAddr.sin_port = htons(port);
    memset(remoteAddr.sin_zero, '\0', sizeof(remoteAddr.sin_zero));

    const ssize_t size =
            sendto(socket_, data, len, MSG_DONTWAIT, (struct sockaddr*)&remoteAddr, sizeof(remoteAddr));

    return size;
}

const char* UdpSocket::LocalAddress() const
{
    return address_;
}

uint16_t UdpSocket::LocalPort() const
{
    return port_;
}
