#ifndef MOCKUDPSOCKET_H
#define MOCKUDPSOCKET_H

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <cstring>
#include <fstream>
#include <filesystem>

#include "Transport/UDPSocket/UDPSocket.h"

class MockUdpSocket: public UdpSocket
{
public:

    MOCK_METHOD(ssize_t, 
        WriteDatagram, (const std::vector<BYTE>& buf, const std::string& host, uint16_t port), 
        (override));

    MOCK_METHOD(ssize_t,  
        ReadDatagram, (std::vector<BYTE>& buf, size_t max_len, std::string& host, uint16_t* port),
         (override));
};

#endif // MOCKUDPSOCKET_H