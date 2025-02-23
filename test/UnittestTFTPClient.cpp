#include "TFTPClient.h"
#include "TFTPPacket.h"
#include "mock/MockUdpSocket.h"

#include <cstring>
#include <fstream>
#include <filesystem>
#include <memory>

#include <gtest/gtest.h>
#include <gmock/gmock.h>


using ::testing::_;
using ::testing::SetArgReferee;
using ::testing::DoAll;
//using ::testing::SetArgReferee;
using ::testing::Return;
using ::testing::InSequence;
using ::testing::Expectation;
using ::testing::Pointee;

// std::ifstream::pos_type GetFileSize(const char* filename)
// {
//     std::ifstream in(filename, std::ifstream::ate | std::ifstream::binary);
//     return in.tellg();
// }
//Fixture for TFTPClient
class TFTPClientTest : public testing::Test {
    protected:
        MockUdpSocket mock_socket_;
        std::unique_ptr<TFTPClient> tftp_cli_;
        std::string server_addr_ = "1.1.1.1";
        uint16_t port_ = 69;    
        size_t kHeaderSize_;
        size_t kMaxDataSize_;

        void SetUp() 
        { 
            tftp_cli_ = std::make_unique<TFTPClient>(&mock_socket_, server_addr_,  port_);
            kHeaderSize_ = tftp_cli_->GetHeaderSize();
            kMaxDataSize_ = tftp_cli_->GetDataSize();  
        } 

        void TearDown() 
        {} 

        // Helper to create reply packet
        std::vector<BYTE> ReadAllBinaryData(const std::string& fname)
        {
            std::ifstream fin(fname, std::ios::in | std::ios::binary);
            std::vector<BYTE> data((std::istreambuf_iterator<char>(fin)),
                                            std::istreambuf_iterator<char>());
            return data;            
        }

        std::vector<BYTE> CreateServerDataReply(const std::string& fname)
        {   
            size_t max_packet_len = kMaxDataSize_ + kHeaderSize_;
            std::vector<BYTE> reply_packet;
            reply_packet.reserve(max_packet_len);
            //  Expected header
            reply_packet.push_back(0);
            reply_packet.push_back((BYTE)OpCode::DATA);
            reply_packet.push_back(0); //block id major byte
            reply_packet.push_back(1); //block id minor byte
            //  Expected data
            std::fstream data_file(fname, std::ios_base::in | std::ios_base::binary);
            reply_packet.insert(reply_packet.begin() + kHeaderSize_,
               std::istream_iterator<BYTE>(data_file),
               std::istream_iterator<BYTE>());
            return reply_packet;
        }

        // Helper func
        std::vector<BYTE> CreateRequest(const std::string& fname, OpCode request_code)
        {
            RequestPacket req_packet(request_code, fname, "octet");
            std::vector<BYTE> buf = req_packet.ToBigEndianVector();
            // opcode (RRQ/WRQ)
            // buf.push_back(0);
            // buf.push_back(static_cast<char>(request_code));
            // // filename
            // buf.insert(buf.end(), fname.begin(), fname.end());
            // buf.push_back(0);
            // // mode (octet/ascii)
            // std::string mode("octet");
            // buf.insert(buf.end(), mode.begin(), mode.end());
            // buf.push_back(0);
            return buf;
        }
};


TEST_F(TFTPClientTest, ReadRequestSuccess)
{
    //ReadRequest packet
    const std::string fname= "dummy";
    std::vector<BYTE> buf = CreateRequest(fname, OpCode::RRQ);
    
    EXPECT_CALL(mock_socket_, WriteDatagram(buf, server_addr_, port_));
    //Call method
    tftp_cli_->Get(fname);
}

TEST_F(TFTPClientTest, WriteRequestSuccess)
{
    //WriteRequest packet
    const std::string fname= "dummy";
    std::vector<BYTE> buf = CreateRequest(fname, OpCode::WRQ);
  
    EXPECT_CALL(mock_socket_, WriteDatagram(buf, server_addr_, port_));
    //Call method
    tftp_cli_->Put(fname);
}

TEST_F(TFTPClientTest, GetFile500BytesSuccess)
{
    //Read request size 
    const std::string data_fname= "test_data/data_500B.txt"; 
    ASSERT_TRUE(std::filesystem::exists(data_fname));
    const size_t kDataSize = std::filesystem::file_size(data_fname);
    ASSERT_GT(kDataSize, 0);
    ASSERT_LE(kDataSize, kMaxDataSize_);
    ssize_t RRQ_packet_size = 2 + data_fname.size() + 1 + strlen("octet") + 1 ;

    uint16_t block_id = 1;
    //Expected DATA from server
    std::vector data = ReadAllBinaryData(data_fname);
    DataPacket reply_packet (block_id, data);

    //ACK packet
    AckPacket ack_packet(block_id);
    std::vector<BYTE> ack_packet_bytes = ack_packet.ToBigEndianVector();

    //Expectations
    InSequence s;
    //Send read request
    EXPECT_CALL(mock_socket_, WriteDatagram(_, server_addr_, port_)).WillOnce(Return(RRQ_packet_size));
    //Read data
    EXPECT_CALL(mock_socket_, ReadDatagram(_, server_addr_, _))
        .WillOnce
            (DoAll
                (
                SetArgReferee<0>(reply_packet.ToBigEndianVector())
                //SetArrayArgument<0>(&reply_packet[0], &reply_packet[0] + max_packet_len)
                ,Return(kDataSize + kHeaderSize_)
                )
            );
    //Send ACK packet (server port will be changed)
    EXPECT_CALL(mock_socket_, WriteDatagram(ack_packet_bytes, server_addr_, _))
        .WillOnce(Return(ack_packet_bytes.size()));
    
    //Call method
    tftp_cli_->Get(data_fname);
}

TEST_F(TFTPClientTest, GetFile2000BytesSuccess)
{
    const std::string data_fname = "test_data/data_2000B.txt"; 
    ASSERT_TRUE(std::filesystem::exists(data_fname));
    const size_t fileSize = std::filesystem::file_size(data_fname);
    ASSERT_EQ(fileSize, 2000) << "Filesize of " << data_fname << "is not 2000 bytes.";

    //Read request
    ssize_t RRQ_packet_size = 2 + data_fname.size() + 1 + strlen("octet") + 1 ;
    EXPECT_CALL(mock_socket_, WriteDatagram(_, server_addr_, port_)).WillOnce(Return(RRQ_packet_size));

    uint16_t block_id = 1;
    std::vector<std::pair<DataPacket, size_t>> expectedReplies;
    std::fstream fin(data_fname, std::ios_base::in | std::ios_base::binary);
    ASSERT_TRUE(fin.is_open());
    while(!fin.eof())
    {
        // Expected DATA in replies
        std::vector<BYTE> data(kMaxDataSize_);
        fin.read((char*)&data[0], kMaxDataSize_);
        DataPacket reply_packet (block_id, data);
        expectedReplies.push_back({reply_packet, fin.gcount()});
        block_id++;
    }

    ASSERT_EQ(expectedReplies.size(), 4);
    EXPECT_CALL(mock_socket_, ReadDatagram(_, server_addr_, _))
        .WillOnce(DoAll(
                    SetArgReferee<0>(expectedReplies[0].first.ToBigEndianVector()),
                    Return(expectedReplies[0].second + kHeaderSize_)
                    )
            )
        .WillOnce(DoAll(
                    SetArgReferee<0>(expectedReplies[1].first.ToBigEndianVector()),
                    Return(expectedReplies[1].second + kHeaderSize_)
                    )
            )
        .WillOnce(DoAll(
                    SetArgReferee<0>(expectedReplies[2].first.ToBigEndianVector()),
                    Return(expectedReplies[2].second + kHeaderSize_)
                    )
            )
        .WillOnce(DoAll(
                    SetArgReferee<0>(expectedReplies[3].first.ToBigEndianVector()),
                    Return(expectedReplies[3].second + kHeaderSize_)
                    )
        );
    

    //ACK
    for(int i = 0; i < 4; i++)
    {
        AckPacket ack_packet(i + 1); 
        EXPECT_CALL(mock_socket_, WriteDatagram(ack_packet.ToBigEndianVector(), server_addr_, _))
            .WillOnce(Return(ack_packet.ToBigEndianVector().size()));
    }
    // EXPECT_CALL(mock_socket_, WriteDatagram(ack_packets[1].ToBigEndianVector(), server_addr_, _))
    //     .WillOnce(Return(ack_packets[1].ToBigEndianVector().size()));

    // EXPECT_CALL(mock_socket_, WriteDatagram(ack_packets[2].ToBigEndianVector(), server_addr_, _))
    //     .WillOnce(Return(ack_packets[2].ToBigEndianVector().size()));

    // EXPECT_CALL(mock_socket_, WriteDatagram(ack_packets[3].ToBigEndianVector(), server_addr_, _))
    //     .WillOnce(Return(ack_packets[3].ToBigEndianVector().size()));


    //Call method
    tftp_cli_->Get(data_fname); 
}


int main(int argc, char** argv)
{   
    ::testing::InitGoogleTest(&argc, argv);
     ::testing::InitGoogleMock(&argc, argv);
    return RUN_ALL_TESTS();
}
