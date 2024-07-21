#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <cstring>
#include <fstream>
#include "tftp_client.h"    
#include "udp_socket.h"


using ::testing::_;
using ::testing::SetArgReferee;
using ::testing::DoAll;
using ::testing::SetArrayArgument;
using ::testing::Return;
using ::testing::InSequence;
using ::testing::Expectation;
using ::testing::Pointee;

class MockUdpSocket: public UdpSocket
{
public:

    MOCK_METHOD(ssize_t, 
        WriteDatagram, (const std::vector<BYTE>& buf,const std::string& host, uint16_t port), 
        (override));

    MOCK_METHOD(ssize_t,  
        ReadDatagram, (BYTE* data,  size_t max_len,  std::string& host, uint16_t* port),
         //ReadDatagram, (char* data,  size_t max_len,  std::string& host, uint16_t* port),
         (override));
};


std::vector<BYTE> CreateRequestBuffer(const std::string& fname, TFTPClient::OpCode request_code)
{
    std::vector<BYTE> buf;
    // opcode (RRQ/WRQ)
    buf.push_back(0);
    buf.push_back(static_cast<char>(request_code));
    // filename
    buf.insert(buf.end(), fname.begin(), fname.end());
    buf.push_back(0);
    // mode (octet/ascii)
    std::string mode("octet");
    buf.insert(buf.end(), mode.begin(), mode.end());
    buf.push_back(0);
    return buf;
}


//Fixture for TFTPClient
class TFTPClientTest : public testing::Test {
    protected:
        MockUdpSocket mock_socket;
        TFTPClient* tftp_cli;
        std::string server_addr = "1.1.1.1";
        uint16_t port = 69;        

        void SetUp() 
        { 
            tftp_cli = new TFTPClient(&mock_socket, server_addr,  port);
        } 

        void TearDown() 
        {
            delete tftp_cli;
            tftp_cli = nullptr;
        } 
};

TEST_F(TFTPClientTest, CheckReadRequest)
{
    //ReadRequest packet
    const std::string fname= "dummy";
    std::vector<BYTE> buf = CreateRequestBuffer(fname, TFTPClient::OpCode::RRQ);
    
    EXPECT_CALL(mock_socket, WriteDatagram(buf, server_addr, port));
    tftp_cli->Get(fname);
}

TEST_F(TFTPClientTest, CheckWriteRequest)
{
    //WriteRequest packet
    const std::string fname= "dummy";
    std::vector<BYTE> buf = CreateRequestBuffer(fname, TFTPClient::OpCode::WRQ);
  
    EXPECT_CALL(mock_socket, WriteDatagram(buf, server_addr, port));
    tftp_cli->Put(fname);
}

TEST_F(TFTPClientTest, CheckReadSmallFile)
{
    //Read request
    const std::string data_fname= "data_500b.txt"; 
    const size_t kDataSize = 500;
    ssize_t RRQ_packet_size = 2 + data_fname.size() + 1 + strlen("octet") + 1 ;

    //Expected data, requested from server
    const size_t kHeaderSize = tftp_cli->GetHeaderSize();
    const size_t kMaxDataSize = tftp_cli->GetDataSize();
    size_t max_packet_len = kMaxDataSize + kHeaderSize;
    std::vector<BYTE> reply_packet;
    reply_packet.reserve(max_packet_len);
    //Header
    reply_packet.push_back(0);
    reply_packet.push_back((BYTE)TFTPClient::OpCode::DATA);
    reply_packet.push_back(0); //block id major byte
    reply_packet.push_back(1); //block id minor byte
    //Data
    std::fstream data_file(data_fname, std::ios_base::in | std::ios_base::binary);
    ASSERT_TRUE(data_file);
    reply_packet.insert(reply_packet.begin() + kHeaderSize,
               std::istream_iterator<BYTE>(data_file),
               std::istream_iterator<BYTE>());
   
    //ACK packet
    std::vector<BYTE> ack_packet;
    const size_t ack_packet_size = 4;
    ack_packet.reserve(ack_packet_size);
    ack_packet.push_back(0);
    ack_packet.push_back((BYTE)TFTPClient::OpCode::ACK);
    ack_packet.push_back(0); 
    ack_packet.push_back(1); 

    //Expectations
    {
        InSequence s;
        //Send read request
        EXPECT_CALL(mock_socket, WriteDatagram(_, server_addr, port)).WillOnce(Return(RRQ_packet_size));
        //Read data
        EXPECT_CALL(mock_socket, ReadDatagram(_, max_packet_len, server_addr, _))
            .WillOnce
                (DoAll
                    (
                    SetArrayArgument<0>(&reply_packet[0], &reply_packet[0] + max_packet_len)
                    ,Return(kDataSize + kHeaderSize)
                    )
                );
        //Send ACK packet
         EXPECT_CALL(mock_socket, WriteDatagram(ack_packet, server_addr, _)).WillOnce(Return(ack_packet_size));
    }

    tftp_cli->Get(data_fname);
}

TEST_F(TFTPClientTest, CheckReadBigFile)
{
   //...
}

int main(int argc, char** argv)
{   
    ::testing::InitGoogleTest(&argc, argv);
     ::testing::InitGoogleMock(&argc, argv);
    return RUN_ALL_TESTS();
}
