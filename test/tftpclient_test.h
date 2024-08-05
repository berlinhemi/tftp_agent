#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <cstring>
#include <fstream>
#include <filesystem>

#include "tftp_client.h" 
#include "tftp_packet.h"   
#include "udp_socket.h"

using ::testing::_;
using ::testing::SetArgReferee;
using ::testing::DoAll;
//using ::testing::SetArgReferee;
using ::testing::Return;
using ::testing::InSequence;
using ::testing::Expectation;
using ::testing::Pointee;

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
        ReadDatagram, (std::vector<BYTE>& buf,  std::string& host, uint16_t* port),
         //ReadDatagram, (char* data,  size_t max_len,  std::string& host, uint16_t* port),
         (override));
    // MOCK_METHOD(ssize_t,  
    //     ReadDatagram, (BYTE* buffer,  size_t max_len,  std::string& host, uint16_t* port),
    //      //ReadDatagram, (char* data,  size_t max_len,  std::string& host, uint16_t* port),
    //      (override));  
};

//Fixture for TFTPClient
class TFTPClientTest : public testing::Test {
    protected:
        MockUdpSocket mock_socket_;
        TFTPClient* tftp_cli_;
        std::string server_addr_ = "1.1.1.1";
        uint16_t port_ = 69;    
        size_t kHeaderSize_;
        size_t kMaxDataSize_;

        void SetUp() 
        { 
            tftp_cli_ = new TFTPClient(&mock_socket_, server_addr_,  port_);
            kHeaderSize_ = tftp_cli_->GetHeaderSize();
            kMaxDataSize_ = tftp_cli_->GetDataSize();  
        } 

        void TearDown() 
        {
            delete tftp_cli_;
            tftp_cli_ = nullptr;
        } 
};

std::vector<BYTE> CreateRequestBuffer(const std::string& fname, /*TFTPClient::*/OpCode request_code);
//std::ifstream::pos_type GetFileSize(const char* filename);
