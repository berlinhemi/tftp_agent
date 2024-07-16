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

//class MockUdpSocket: public UdpSocket
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

//Move tftp_cli to Fixture (?)

TEST(TFTPCliTest, CheckReadRequest)
{
    MockUdpSocket mock_socket;
    const std::string server_addr = "1.1.1.1";
    uint16_t port = 69;
    TFTPClient tftp_cli(&mock_socket, server_addr,  port);
    
    //RRQ
    const std::string fname= "data.txt";
    std::vector<BYTE> buf = CreateRequestBuffer(fname, TFTPClient::OpCode::RRQ);
    
    
    EXPECT_CALL(mock_socket, WriteDatagram(buf, server_addr, port));
    tftp_cli.Get(fname);
    
}

TEST(TFTPCliTest, CheckWriteRequest)
{
    MockUdpSocket mock_socket;
    const std::string server_addr = "1.1.1.1";
    uint16_t port = 69;
    TFTPClient tftp_cli(&mock_socket, server_addr,  port);

    //WRQ
    const std::string fname= "data.txt";
    std::vector<BYTE> buf = CreateRequestBuffer(fname, TFTPClient::OpCode::WRQ);
  
    
    EXPECT_CALL(mock_socket, WriteDatagram(buf, server_addr, port));
    tftp_cli.Put(fname);
    
}

TEST(TFTPCliTest, CheckReadFile)
{
    MockUdpSocket mock_socket;
    std::string server_addr = "1.1.1.1";
    uint16_t port = 69;
    TFTPClient tftp_cli(&mock_socket, server_addr,  port);

    //Read request
    const std::string target_fname= "data.txt";  
    ssize_t RRQ_packet_size = 2 + target_fname.size() + 1 + strlen("octet") + 1 ;
    //Expectation rrq_expectation = 
        //EXPECT_CALL(mock_socket, WriteDatagram(_, server_addr, port)).Return(RRQ_packet_size);
    

    //Reply data
    const std::string data_fname= "data_500b.txt";
    const size_t kHeaderSize = 4;
    const size_t kMaxDataSize = 512;
    const size_t kFileSize = 500;
    size_t max_packet_len = kMaxDataSize + kHeaderSize;
    std::vector<BYTE> buffer;
    buffer.reserve(max_packet_len);
    std::fstream file(data_fname, std::ios_base::in | std::ios_base::binary);
    ASSERT_TRUE(file);
    //  if (!file) {
    //     std::puts("\nError! Can't open file for reading!");
    //     return;
    // }
    buffer.push_back(0);
    buffer.push_back((BYTE)TFTPClient::OpCode::DATA);
    buffer.push_back(0); //block id
    buffer.push_back(1); 
       /// ==> FORMULA !!!
     //received_block_ = ((uint8_t)buffer_[2] << 8) | (uint8_t)buffer_[3]; 
  
    buffer.insert(buffer.begin() + kHeaderSize,
               std::istream_iterator<BYTE>(file),
               std::istream_iterator<BYTE>());
   

    //ACK
    std::vector<BYTE> ack_buffer;
    const size_t ack_packet_size = 4;
    ack_buffer.reserve(ack_packet_size);
   
    ack_buffer.push_back(0);
    ack_buffer.push_back((BYTE)TFTPClient::OpCode::ACK);
    ack_buffer.push_back(0); //block id
    ack_buffer.push_back(1); 

    //std::cout << buffer[4] << std::endl;

    //std::vector<BYTE> data;
    // std::vector<BYTE> data;
    // data.reserve(max_len);
    //char* data;
    //char* buffer2;
    {
        InSequence s;

        EXPECT_CALL(mock_socket, WriteDatagram(_, server_addr, port)).WillOnce(Return(RRQ_packet_size));

        EXPECT_CALL(mock_socket, 
        ReadDatagram(_, max_packet_len, server_addr, _))
        // ReadDatagram(&data[0], max_len, server_addr, _))
        
           // .After(rrq_expectation)
        // .WillOnce(Return(max_len))
        //.WillOnce(SetArrayArgument<0>(&buffer[0], &buffer[0] + max_len))
        //.WillOnce(SetArrayArgument<0>(buffer2, buffer2 + max_len))
            .WillOnce(DoAll(
                SetArrayArgument<0>(&buffer[0], &buffer[0] + max_packet_len)
                ,Return(kFileSize + kHeaderSize)
                ))
        ;

         EXPECT_CALL(mock_socket, WriteDatagram(ack_buffer, server_addr, _)).WillOnce(Return(ack_packet_size));
    }
    tftp_cli.Get(target_fname);
    
}

int main(int argc, char** argv)
{   
    ::testing::InitGoogleTest(&argc, argv);
     ::testing::InitGoogleMock(&argc, argv);
    return RUN_ALL_TESTS();
}
