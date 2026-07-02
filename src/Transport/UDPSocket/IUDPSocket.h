#ifndef IUDP_SOCKET_H
#define IUDP_SOCKET_H

#include <vector>
#include <string>
#include <stdio.h>
#include <cstdint>

typedef unsigned char BYTE;

/**
 ?????
 */
class IUdpSocket
{
public:

    virtual ssize_t ReadDatagram(std::vector<BYTE>& buffer, 
                                size_t max_len,
                                std::string& host,
                                uint16_t* port) = 0;
          
    virtual ssize_t WriteDatagram(const std::vector<BYTE>& data,
                                const std::string& host,
                                uint16_t port) = 0;

    
    virtual bool IsInitialized() = 0;



};

#endif // IUDP_SOCKET_H