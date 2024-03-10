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

    bool InitSocket();
    bool BindSocket(const char *localAddress, uint16_t localPort);
    bool BindSocket(uint16_t localPort);
    void Abort();

    int64_t ReadDatagram(char *data, int64_t maxlen, char *host = nullptr, uint16_t *port = nullptr);
    int64_t WriteDatagram(const char *data, int64_t len, const char *host, uint16_t port);

    const char *LocalAddress() const;
    uint16_t LocalPort() const;

private:
    //max: 4 octets + 3 dots
    char m_address[15];
    uint16_t m_port;
    int m_socket;
// #ifdef WIN32
//     size_t m_socket;
// #else
    
//#endif
};

#endif // UDP_SOCKET_H