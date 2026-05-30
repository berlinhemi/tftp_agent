#ifndef UDP_SOCKET_H
#define UDP_SOCKET_H

#include <vector>
#include <string>
#include <stdio.h>
#include <cstdint>

typedef unsigned char BYTE;

class UdpSocket
{
public:
    UdpSocket();

    bool IsInitialized();

    virtual ssize_t ReadDatagram(
        std::vector<BYTE>& buffer, 
        size_t max_len,
        std::string& host,
        uint16_t* port);
        
    virtual ssize_t WriteDatagram(
        const std::vector<BYTE>& data,
        const std::string& host,
        uint16_t port);

    ~UdpSocket();

private:

    int m_socket {-1};
    bool m_initialized {false};
    bool Init();
    void Abort();
};

#endif // UDP_SOCKET_H