#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <cstring>
#include "tftp_client.h"    
#include "udp_socket.h"


using ::testing::_;
using ::testing::SetArgReferee;
using ::testing::DoAll;
using ::testing::Return;

//class MockUdpSocket: public UdpSocket
class MockUdpSocket: public UdpSocket
{
public:

    MOCK_METHOD(ssize_t, 
        WriteDatagram, (const std::vector<BYTE>& buf,const std::string& host, uint16_t port), 
        (override));
    //MOCK_METHOD(ssize_t,  WriteDatagram, (const char *data, size_t len, const char *host, uint16_t port));

};


std::vector<BYTE> CreateRequestBuffer(const std::string& fname, TFTPClient::OpCode request_code)
{
    std::vector<BYTE> buf;
    buf.push_back(0);
    buf.push_back(static_cast<char>(request_code));
    // filename
    buf.insert(buf.end(), fname.begin(), fname.end());
    buf.push_back(0);
    // mode
    std::string mode("octet");
    buf.insert(buf.end(), mode.begin(), mode.end());
    buf.push_back(0);
    return buf;
}

TEST(TFTPCliTest, CheckReadRequest)
{
    MockUdpSocket mock_socket;
    const std::string server_addr = "1.1.1.1";
    uint16_t port = 69;
    TFTPClient tftp_cli(&mock_socket, server_addr,  port);

    const std::string fname= "data.txt";
    std::vector<BYTE> buf = CreateRequestBuffer(fname, TFTPClient::OpCode::RRQ);
    
    //RRQ
    EXPECT_CALL(mock_socket, WriteDatagram(buf, server_addr, port));
    tftp_cli.Get(fname);
    
}

TEST(TFTPCliTest, CheckWriteRequest)
{
    MockUdpSocket mock_socket;
    const std::string server_addr = "1.1.1.1";
    uint16_t port = 69;
    TFTPClient tftp_cli(&mock_socket, server_addr,  port);

    const std::string fname= "data.txt";
    std::vector<BYTE> buf = CreateRequestBuffer(fname, TFTPClient::OpCode::WRQ);
  
    //WRQ
    EXPECT_CALL(mock_socket, WriteDatagram(buf, server_addr, port));
    tftp_cli.Put(fname);
    
}


int main(int argc, char** argv)
{   
    ::testing::InitGoogleTest(&argc, argv);
     ::testing::InitGoogleMock(&argc, argv);
    return RUN_ALL_TESTS();
}