#include "tftpclient_test.h"



// std::ifstream::pos_type GetFileSize(const char* filename)
// {
//     std::ifstream in(filename, std::ifstream::ate | std::ifstream::binary);
//     return in.tellg();
// }

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


TEST_F(TFTPClientTest, CheckReadRequest)
{
    //ReadRequest packet
    const std::string fname= "dummy";
    std::vector<BYTE> buf = CreateRequestBuffer(fname, TFTPClient::OpCode::RRQ);
    
    EXPECT_CALL(mock_socket_, WriteDatagram(buf, server_addr_, port_));
    tftp_cli_->Get(fname);
}

TEST_F(TFTPClientTest, CheckWriteRequest)
{
    //WriteRequest packet
    const std::string fname= "dummy";
    std::vector<BYTE> buf = CreateRequestBuffer(fname, TFTPClient::OpCode::WRQ);
  
    EXPECT_CALL(mock_socket_, WriteDatagram(buf, server_addr_, port_));
    tftp_cli_->Put(fname);
}

TEST_F(TFTPClientTest, CheckReadSmallFile)
{

    //Read request size 
    const std::string data_fname= "test_data/data_500b.txt"; 
    ASSERT_TRUE(std::filesystem::exists(data_fname));
    const size_t kDataSize = std::filesystem::file_size(data_fname);
    ASSERT_GT(kDataSize, 0);
    ASSERT_LE(kDataSize, kMaxDataSize_);
    ssize_t RRQ_packet_size = 2 + data_fname.size() + 1 + strlen("octet") + 1 ;

    //Expected data, requested from server
    size_t max_packet_len = kMaxDataSize_ + kHeaderSize_;
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
    reply_packet.insert(reply_packet.begin() + kHeaderSize_,
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
        //Send read request (detect size only)
        EXPECT_CALL(mock_socket_, WriteDatagram(_, server_addr_, port_)).WillOnce(Return(RRQ_packet_size));
        //Read data
        EXPECT_CALL(mock_socket_, ReadDatagram(_, max_packet_len, server_addr_, _))
            .WillOnce
                (DoAll
                    (
                    //argument <0> - container for receiving data
                    SetArrayArgument<0>(&reply_packet[0], &reply_packet[0] + max_packet_len)
                    ,Return(kDataSize + kHeaderSize_)
                    )
                );
        //Send ACK packet
         EXPECT_CALL(mock_socket_, WriteDatagram(ack_packet, server_addr_, _)).WillOnce(Return(ack_packet_size));
    }

    tftp_cli_->Get(data_fname);
}

TEST_F(TFTPClientTest, CheckRead5KBFile)
{
    //Read request size 
    const std::string data_fname= "test_data/data_5kb.txt"; 
    ASSERT_TRUE(std::filesystem::exists(data_fname));
    const size_t kDataSize = std::filesystem::file_size(data_fname);
    ASSERT_GT(kDataSize, 0);
    ssize_t RRQ_packet_size = 2 + data_fname.size() + 1 + strlen("octet") + 1 ;

    
}




int main(int argc, char** argv)
{   
    ::testing::InitGoogleTest(&argc, argv);
     ::testing::InitGoogleMock(&argc, argv);
    return RUN_ALL_TESTS();
}
