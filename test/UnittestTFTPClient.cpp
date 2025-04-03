#include "TFTPClient.h"
#include "TFTPPacketTypes.h"
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
using ::testing::Invoke;

#include "easylogging++.h"
// Define it only once in project
INITIALIZE_EASYLOGGINGPP 

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
            el::Loggers::reconfigureAllLoggers(el::ConfigurationType::Format, "[%level] %msg");

            tftp_cli_ = std::make_unique<TFTPClient>(&mock_socket_, server_addr_,  port_);
            kHeaderSize_ = tftp_cli_->GetHeaderSize();
            kMaxDataSize_ = tftp_cli_->GetDataSize();  
        } 

        void TearDown() {} 

        /*
            @brief Helper function for reading binary file
        */ 
        std::vector<BYTE> ReadAllBinaryData(const std::string& fname)
        {
            std::ifstream fin(fname, std::ios::in | std::ios::binary);
            std::vector<BYTE> data((std::istreambuf_iterator<char>(fin)),
                                            std::istreambuf_iterator<char>());
            return data;            
        }

        /*
            @brief Helper function for creating reply packet from server
        */ 
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

        /*
            @brief Helper function for creating request packet
        */ 
        std::vector<BYTE> CreateRequest(const std::string& fname, OpCode request_code)
        {
            RequestPacket req_packet(request_code, fname, "octet");
            std::vector<BYTE> buf = req_packet.ToBigEndianVector();
            return buf;
        }
};

// /*
//     @brief Test of Get method
//         when SendRequest failed since WriteDatagram returns zero
// */
// TEST_F(TFTPClientTest, Get_SendRequestReturnsError_Failed)
// {
//     // ReadRequest packet
//     const std::string fname= "somefile";
//     std::vector<BYTE> buf = CreateRequest(fname, OpCode::RRQ);
    
//     EXPECT_CALL(mock_socket_, WriteDatagram(buf, server_addr_, port_))
//             .WillOnce(Return(0));
//     // Call method
//     EXPECT_EQ(tftp_cli_->Get(fname), TFTPClient::Status::kWriteError);
// }

// /*
//     @brief Test of Put method
//         when SendRequest failed since WriteDatagram returns zero
// */
// TEST_F(TFTPClientTest, Put_SendRequestReturnsError_Failed)
// {
//     // WriteRequest packet
//     const std::string fname= "test_data/data_5KB.txt";
//     std::vector<BYTE> buf = CreateRequest(fname, OpCode::WRQ);
  
//     EXPECT_CALL(mock_socket_, WriteDatagram(buf, server_addr_, port_))
//             .WillOnce(Return(0));
//     // Call method
//     EXPECT_EQ(tftp_cli_->Put(fname), TFTPClient::Status::kWriteError);
// }

// /*
//     @brief Test of Put method
//         when SendRequest and GetFile successed 
//         while reading 500 bytes
// */
// TEST_F(TFTPClientTest, Get_ReadFile500Bytes_Success)
// {
//     const std::string fname= "test_data/data_500B.txt"; 
//     ASSERT_TRUE(std::filesystem::exists(fname));
//     const size_t kDataSize = std::filesystem::file_size(fname);
//     // Check filesize
//     ASSERT_GT(kDataSize, 0);
//     ASSERT_LE(kDataSize, kMaxDataSize_);
//     // Read request size 
//     ssize_t RRQ_packet_size = 2 + fname.size() + 1 + strlen("octet") + 1 ;

//     uint16_t block_id = 1;
//     // Expected DATA from server
//     std::vector expected_data = ReadAllBinaryData(fname);
//     DataPacket reply_packet (block_id, expected_data);

//     // ACK packet
//     AckPacket ack_packet(block_id);
//     std::vector<BYTE> ack_packet_bytes = ack_packet.ToBigEndianVector();

//     // Expectations
//     InSequence s;
//     // Send read request
//     EXPECT_CALL(mock_socket_, WriteDatagram(_, server_addr_, port_)).WillOnce(Return(RRQ_packet_size));
//     // Read data in one step
//     EXPECT_CALL(mock_socket_, ReadDatagram(_, server_addr_, _))
//         .WillOnce
//             (DoAll
//                 (
//                 SetArgReferee<0>(reply_packet.ToBigEndianVector()),
//                 Return(kDataSize + kHeaderSize_)
//                 )
//             );
//     // Send ACK packet (server port will be changed)
//     EXPECT_CALL(mock_socket_, WriteDatagram(ack_packet_bytes, server_addr_, _))
//         .WillOnce(Return(ack_packet_bytes.size()));
    
//     // Call method
//     EXPECT_EQ(tftp_cli_->Get(fname), TFTPClient::Status::kSuccess);

// }

// /*
//     @brief Test of Put method
//         when SendRequest and GetFile successed 
//         while reading 2000 bytes
// */
// TEST_F(TFTPClientTest, GetFile2000BytesSuccess)
// {
//     const std::string fname = "test_data/data_2000B.txt"; 
//     ASSERT_TRUE(std::filesystem::exists(fname));
//     const size_t fileSize = std::filesystem::file_size(fname);
//     ASSERT_EQ(fileSize, 2000) << "Filesize of " << fname << "is not 2000 bytes.";

//     // Read request
//     ssize_t RRQ_packet_size = 2 + fname.size() + 1 + strlen("octet") + 1 ;
//     EXPECT_CALL(mock_socket_, WriteDatagram(_, server_addr_, port_)).WillOnce(Return(RRQ_packet_size));

//     uint16_t block_id = 1;
//     // Expected replies from server
//     std::vector<std::pair<DataPacket, size_t>> expectedReplies; // ermove size_t?
//     std::fstream fin(fname, std::ios_base::in | std::ios_base::binary);
//     ASSERT_TRUE(fin.is_open());
//     while(!fin.eof())
//     {
//         std::vector<BYTE> data(kMaxDataSize_);
//         // Read kMaxDataSize_ bytes of data 
//         fin.read((char*)&data[0], kMaxDataSize_);
//         data.resize(fin.gcount()); // Check it
//         DataPacket reply_packet (block_id, data);
//         expectedReplies.push_back({reply_packet, fin.gcount()});
//         block_id++;
//     }

//     // fileSize / kMaxDataSize_ equals 4
//     ASSERT_EQ(expectedReplies.size(), 4);
//     // Read data in four steps
//     EXPECT_CALL(mock_socket_, ReadDatagram(_, server_addr_, _))
//         .WillOnce(DoAll(
//                     SetArgReferee<0>(expectedReplies[0].first.ToBigEndianVector()),
//                     Return(expectedReplies[0].second + kHeaderSize_)
//                     )
//             )
//         .WillOnce(DoAll(
//                     SetArgReferee<0>(expectedReplies[1].first.ToBigEndianVector()),
//                     Return(expectedReplies[1].second + kHeaderSize_)
//                     )
//             )
//         .WillOnce(DoAll(
//                     SetArgReferee<0>(expectedReplies[2].first.ToBigEndianVector()),
//                     Return(expectedReplies[2].second + kHeaderSize_)
//                     )
//             )
//         .WillOnce(DoAll(
//                     SetArgReferee<0>(expectedReplies[3].first.ToBigEndianVector()),
//                     Return(expectedReplies[3].second + kHeaderSize_)
//                     )
//         );
    

//     // ACK pacets
//     for(uint16_t packet_id = 1; packet_id <= 4; packet_id++)
//     {
//         AckPacket ack_packet(packet_id); 
//         EXPECT_CALL(mock_socket_, WriteDatagram(ack_packet.ToBigEndianVector(), server_addr_, _))
//             .WillOnce(Return(ack_packet.ToBigEndianVector().size()));
//     }


//     // Call method
//     EXPECT_EQ(tftp_cli_->Get(fname), TFTPClient::Status::kSuccess);
// }


int main(int argc, char** argv)
{   
    ::testing::InitGoogleTest(&argc, argv);
    ::testing::InitGoogleMock(&argc, argv);
    return RUN_ALL_TESTS();
}
