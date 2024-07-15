#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <cstring>
#include <fstream>
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

    MOCK_METHOD(ssize_t,  
        ReadDatagram, (std::vector<BYTE>& data,  size_t max_len,  std::string& host, uint16_t* port),
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

    //RRQ
    const std::string target_fname= "data.txt";  
    ssize_t RRQ_packet_size = 2 + target_fname.size() + 1 + strlen("octet") + 1 ;
    EXPECT_CALL(mock_socket, WriteDatagram(_, server_addr, port)).WillOnce(Return(RRQ_packet_size));
    

    
    const std::string data_fname= "data_512b.txt";
    const int kHeaderSize = 4;
    const int kDataSize = 512;
    std::vector<BYTE> buffer;
    buffer.reserve(kHeaderSize + kDataSize);
    std::cout << "size1: " << buffer.size() << std::endl;

    std::fstream file(data_fname, std::ios_base::in | std::ios_base::binary);
    ASSERT_TRUE(file);
     if (!file) {
        std::puts("\nError! Can't open file for reading!");
        return;
    }
    
    buffer.push_back(0);
    buffer.push_back((BYTE)TFTPClient::OpCode::DATA);
    buffer.push_back(0);
    buffer.push_back(0);
    
     std::cout <<  "size2: " << buffer.size() << std::endl;
     buffer.insert(buffer.begin(),
               std::istream_iterator<BYTE>(file),
               std::istream_iterator<BYTE>());
    //file.read((char*)&(buffer[0]) + kHeaderSize, kDataSize);
    //received_block_ = ((uint8_t)buffer_[2] << 8) | (uint8_t)buffer_[3]; ==> FORMULA
    std::cout << "OK" << std::endl;
    std::cout <<  "size3: " << buffer.size() << std::endl;
    std::cout << buffer[4] << std::endl;
    std::cout << buffer[5] << std::endl;

    size_t max_len = kDataSize + kHeaderSize;
    EXPECT_CALL(mock_socket, ReadDatagram(buffer, max_len, server_addr, _)).WillOnce(Return(max_len));

    tftp_cli.Get(target_fname);
    
}

int main(int argc, char** argv)
{   
    ::testing::InitGoogleTest(&argc, argv);
     ::testing::InitGoogleMock(&argc, argv);
    return RUN_ALL_TESTS();
}
