#ifndef UDP_SOCKET_H
#define UDP_SOCKET_H

#include <vector>
#include <string>
#include <stdio.h>
#include <cstdint>

typedef unsigned char BYTE;
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
    //bool Bind(const char *local_address, uint16_t local_port);
    //bool Bind(uint16_t local_port);
    bool IsInitialized();

   // virtual ssize_t ReadDatagram(char *data, size_t max_len, char *host = nullptr, uint16_t *port = nullptr);
    virtual ssize_t ReadDatagram( //BYTE* buffer,
        std::vector<BYTE>& data, 
        //size_t max_len,
        std::string& host,
        uint16_t* port);
        
    virtual ssize_t WriteDatagram(
        //const BYTE* data,
        //size_t data_len,
        const std::vector<BYTE>& data,
        const std::string& host,
        uint16_t port);
    //irtual ssize_t WriteDatagram(const char *data, size_t len, const char *host, uint16_t port);

    //const char *LocalAddress() const;
    //uint16_t LocalPort() const;
    ~UdpSocket();

private:
    //max: 4 octets (4*3) + 3 dots 
    //char address_[15];
    //uint16_t port_;
    int socket_ {-1};
    bool initialized_ {false};
    bool Init();
    void Abort();
};

#endif // UDP_SOCKET_H