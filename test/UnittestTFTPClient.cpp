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

/*
    @brief Fixture for TFTPClient
*/ 
class TFTPClientTest : public testing::Test {
    protected:

        MockUdpSocket m_mock_socket;
        std::unique_ptr<TFTPClient> m_tftp_client;
        std::string m_server_addr = "1.1.1.1";
        uint16_t m_port = 69;    
        size_t m_header_size;
        size_t m_max_data_size;

        void SetUp() 
        { 
            el::Loggers::reconfigureAllLoggers(el::ConfigurationType::Format, "[%level] %msg");
            el::Loggers::reconfigureAllLoggers(el::ConfigurationType::ToFile, "false");

            m_tftp_client = std::make_unique<TFTPClient>(&m_mock_socket, m_server_addr,  m_port);
            m_header_size = TFTPClient::GetHeaderSize();
            m_max_data_size = TFTPClient::GetDataSize();  
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
            size_t max_packet_len = m_header_size + m_max_data_size;
            std::vector<BYTE> reply_packet;
            reply_packet.reserve(max_packet_len);
            //  Expected header
            reply_packet.push_back(0);
            reply_packet.push_back((BYTE)OpCode::DATA);
            reply_packet.push_back(0); //block id major byte
            reply_packet.push_back(1); //block id minor byte
            //  Expected data
            std::fstream data_file(fname, std::ios_base::in | std::ios_base::binary);
            reply_packet.insert(reply_packet.begin() + m_header_size,
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

/*
    @brief Test of Get method
        when SendRequest failed since WriteDatagram returns zero
*/
TEST_F(TFTPClientTest, Get_SendRequestReturnsError_Failed)
{
    // ReadRequest packet
    std::string fname = TFTPClient::GetCommandFName();
    std::vector<BYTE> req = CreateRequest(fname, OpCode::RRQ);
    
    EXPECT_CALL(m_mock_socket, WriteDatagram(req, m_server_addr, m_port))
            .WillOnce(Return(0));
    // Call method
    std::vector<BYTE> buffer;
    EXPECT_EQ(m_tftp_client->GetCommand(buffer), TFTPClient::Status::kSendRequestError);
}

/*
    @brief Test of Put method
        when SendRequest failed since WriteDatagram returns zero
*/
TEST_F(TFTPClientTest, Put_SendRequestReturnsError_Failed)
{
    // WriteRequest packet
    std::string fname = TFTPClient::GetResultFName();
    std::vector<BYTE> req = CreateRequest(fname, OpCode::WRQ);
  
    EXPECT_CALL(m_mock_socket, WriteDatagram(req, m_server_addr, m_port))
            .WillOnce(Return(0));
    
    // Call method
    std::vector<BYTE> buffer {1, 2, 3};
    EXPECT_EQ(m_tftp_client->PutResults(buffer), TFTPClient::Status::kSendRequestError);
}

/*
    @brief Test of Put method
        when SendRequest and GetFile successed 
        while reading 512-4 bytes of payload
*/
TEST_F(TFTPClientTest, Get_ReadFileMaxPacketSize_Success)
{
    const std::string fname = "test_data/data_508B.bin";
    ASSERT_TRUE(std::filesystem::exists(fname));
    size_t data_size = std::filesystem::file_size(fname);
    // Check filesize
    ASSERT_GT(data_size, 0);
    ASSERT_LE(data_size, m_max_data_size);
    
    const uint16_t block_id = 1;

    // Set expectations
    InSequence s;
    // Send read request
    ssize_t RRQ_packet_size = 2 +  TFTPClient::GetCommandFName().size() + 1 + strlen("octet") + 1 ;
    EXPECT_CALL(m_mock_socket, WriteDatagram(_, m_server_addr, m_port)).WillOnce(Return(RRQ_packet_size));
    // Read data in one packet
    std::vector expected_data = ReadAllBinaryData(fname);
    DataPacket reply_packet (block_id, expected_data);
    EXPECT_CALL(m_mock_socket, ReadDatagram(_, _, m_server_addr, _))
        .WillOnce
            (DoAll
                (
                SetArgReferee<0>(reply_packet.ToBigEndianVector()),
                Return(data_size + m_header_size)
                )
            );

    // Send ACK packet (server port will be changed)
    AckPacket ack_packet(block_id);
    std::vector<BYTE> ack_packet_bytes = ack_packet.ToBigEndianVector();
    EXPECT_CALL(m_mock_socket, WriteDatagram(ack_packet_bytes, m_server_addr, _))
        .WillOnce(Return(ack_packet_bytes.size()));
    
    // Call method
    std::vector<BYTE> buffer;
    EXPECT_EQ(m_tftp_client->GetCommand(buffer), TFTPClient::Status::kSuccess);
    // Check results
    EXPECT_EQ(buffer, expected_data);
}

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
