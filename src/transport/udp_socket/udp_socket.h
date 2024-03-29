#ifndef UDP_SOCKET_H
#define UDP_SOCKET_H

#include <stdio.h>
#include <cstdint>

/* Socket timeout
 *
    // LINUX
    struct timeval tv;
    tv.tv_sec = timeout_in_seconds;
    tv.tv_usec = 0;
    setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof tv);

    // WINDOWS
    DWORD timeout = timeout_in_seconds * 1000;
    setsockopt(socket, SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeout, sizeof timeout);

    // MAC OS X (identical to Linux)
    struct timeval tv;
    tv.tv_sec = timeout_in_seconds;
    tv.tv_usec = 0;
    setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof tv);
 *
*/

class UdpSocket
{
public:
    UdpSocket();
    ~UdpSocket();

    bool Init();
    bool Bind(const char *local_address, uint16_t local_port);
    bool Bind(uint16_t local_port);
    void Abort();

    int64_t ReadDatagram(char *data, int64_t max_len, char *host = nullptr, uint16_t *port = nullptr);
    int64_t WriteDatagram(const char *data, int64_t len, const char *host, uint16_t port);

    const char *LocalAddress() const;
    uint16_t LocalPort() const;

private:
    //max: 4 octets (4*3) + 3 dots 
    char address_[15];
    uint16_t port_;
    int socket_;

};

#endif // UDP_SOCKET_H