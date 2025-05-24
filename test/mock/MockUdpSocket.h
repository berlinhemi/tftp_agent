#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <cstring>
#include <fstream>
#include <filesystem>

//#include "tftp_client.h" 
//#include "tftp_packet.h"   
#include "UDPSocket/UDPSocket.h"

// using ::testing::_;
// using ::testing::SetArgReferee;
// using ::testing::DoAll;
// //using ::testing::SetArgReferee;
// using ::testing::Return;
// using ::testing::InSequence;
// using ::testing::Expectation;
// using ::testing::Pointee;

class MockUdpSocket: public UdpSocket
{
public:

    MOCK_METHOD(ssize_t, 
        WriteDatagram, (const std::vector<BYTE>& buf, const std::string& host, uint16_t port), 
        (override));
    // MOCK_METHOD(ssize_t, 
    //     WriteDatagram, (const BYTE* data, size_t data_len, const std::string& host, uint16_t port), 
    //     (override));

    MOCK_METHOD(ssize_t,  

        ReadDatagram, (std::vector<BYTE>& buf, size_t max_len, std::string& host, uint16_t* port),
         //ReadDatagram, (char* data,  size_t max_len,  std::string& host, uint16_t* port),
         (override));
    // MOCK_METHOD(ssize_t,  
    //     ReadDatagram, (BYTE* buffer,  size_t max_len,  std::string& host, uint16_t* port),
    //      //ReadDatagram, (char* data,  size_t max_len,  std::string& host, uint16_t* port),
    //      (override));  
};
