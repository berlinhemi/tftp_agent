#include "TFTPClient.h"
#include "TFTPPacketTypes.h"
#include "mock/MockUdpSocket.h"

#include <cstring>
#include <fstream>
#include <filesystem>
#include <memory>

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "easylogging++.h"
#include "easylogging++.cc"

using ::testing::_;
using ::testing::SetArgReferee;
using ::testing::DoAll;
//using ::testing::SetArgReferee;
using ::testing::Return;
using ::testing::InSequence;
using ::testing::Expectation;
using ::testing::Pointee;
using ::testing::Invoke;


// Define it only once in project
INITIALIZE_EASYLOGGINGPP 

/*
    @brief Fixture for TFTPClient
*/
class TFTPClientTest : public testing::Test
{
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
        el::Loggers::setVerboseLevel(3);

        m_tftp_client = std::make_unique<TFTPClient>(&m_mock_socket, m_server_addr, m_port);
        m_header_size = TFTPClient::GetHeaderSize();
        m_max_data_size = TFTPClient::GetMaxDataSize();
    }

    void TearDown() {}

    /*
        @brief Helper function for reading binary file
    */
    std::vector<BYTE> ReadAllBinaryData(const std::string &fname)
    {
        std::ifstream fin(fname, std::ios::in | std::ios::binary);
        std::vector<BYTE> data((std::istreambuf_iterator<char>(fin)),
                               std::istreambuf_iterator<char>());
        return data;
    }

    /*
        @brief Helper function for creating reply packet from server
    */
    std::vector<BYTE> CreateServerDataReply(const std::string &fname)
    {
        size_t max_packet_len = m_header_size + m_max_data_size;
        std::vector<BYTE> reply_packet;
        reply_packet.reserve(max_packet_len);
        //  Expected header
        reply_packet.push_back(0);
        reply_packet.push_back((BYTE)OpCode::DATA);
        reply_packet.push_back(0); // block id major byte
        reply_packet.push_back(1); // block id minor byte
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
    std::vector<BYTE> CreateRequest(const std::string &fname, OpCode request_code)
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
TEST_F(TFTPClientTest, GetCommand_SendRequestReturnsError_Failed)
{
    // ReadRequest packet
    std::string fname = TFTPClient::GetDownloadedDefaultFName();
    std::vector<BYTE> req = CreateRequest(fname, OpCode::RRQ);
    
    EXPECT_CALL(m_mock_socket, WriteDatagram(req, m_server_addr, m_port))
            .WillOnce(Return(0));
    // Call method
    std::vector<BYTE> buffer;
    EXPECT_EQ(m_tftp_client->Get(buffer, fname), TFTPClient::Status::kSendRequestError);
}


/*
    @brief Test of Get method
        when SendRequest and GetFile successed 
        while reading less than 512 bytes of payload
*/
TEST_F(TFTPClientTest, GetCommand_ReadDataSmallerThanMaxDataSize_Success)
{
    const std::string input_fname = "test_data/data_500B.bin";
    ASSERT_TRUE(std::filesystem::exists(input_fname));
    size_t data_size = std::filesystem::file_size(input_fname);
    // Check filesize
    ASSERT_GT(data_size, 0);
    ASSERT_LE(data_size, m_max_data_size) << "Filesize of " << input_fname 
            << "is bigger than " << m_max_data_size << " bytes.";
    
    const uint16_t block_id = 1;

    // Set expectations
    InSequence s;
    // Send read request
    const std::string fname = TFTPClient::GetDownloadedDefaultFName();
    ssize_t RRQ_packet_size = 2 + fname.size() + 1 + strlen("octet") + 1;
    EXPECT_CALL(m_mock_socket, WriteDatagram(_, m_server_addr, m_port)).WillOnce(Return(RRQ_packet_size));
    // Read data in one packet
    std::vector expected_data = ReadAllBinaryData(input_fname);
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
    EXPECT_EQ(m_tftp_client->Get(buffer, fname), TFTPClient::Status::kSuccess);
    // Check results
    EXPECT_EQ(buffer, expected_data);
}

/*
    @brief Test of Get method
        when SendRequest and GetFile successed 
        while reading more than 512 bytes
*/
TEST_F(TFTPClientTest, GetCommand_Read2000Bytes_Success)
{
    const std::string input_fname = "test_data/data_2000B.txt"; 
    ASSERT_TRUE(std::filesystem::exists(input_fname));
    const size_t file_size = std::filesystem::file_size(input_fname);
    ASSERT_EQ(file_size, 2000) << "Filesize of " << input_fname << "is not 2000 bytes.";

    // Read request
    const std::string fname = TFTPClient::GetDownloadedDefaultFName();
    ssize_t RRQ_packet_size = 2 + fname.size() + 1 + strlen("octet") + 1 ;
    EXPECT_CALL(m_mock_socket, WriteDatagram(_, m_server_addr, m_port)).WillOnce(Return(RRQ_packet_size));

    
    std::fstream fin(input_fname, std::ios_base::in | std::ios_base::binary);
    ASSERT_TRUE(fin.is_open());
    InSequence seq;
    uint16_t block_id = 1;
    while(!fin.eof())
    {
        ASSERT_LT(block_id, 5) << "Too many data chunks have been read"; // 2000 bytes will be sent in 4 packets
        std::vector<BYTE> data(m_max_data_size);
        // Read kMaxDataSize_ bytes of data 
        fin.read((char*)&data[0], m_max_data_size);
        data.resize(fin.gcount()); // Adjust size 
        
        DataPacket reply_packet (block_id, data);
        // Read data expectation
        EXPECT_CALL(m_mock_socket, ReadDatagram(_, _, m_server_addr, _))
            .WillOnce(DoAll(
                        SetArgReferee<0>(reply_packet.ToBigEndianVector()),
                        Return(reply_packet.PacketSize())
                    ));

        AckPacket ack_packet(block_id); 
        // Send ACK expectation
        EXPECT_CALL(m_mock_socket, WriteDatagram(ack_packet.ToBigEndianVector(), m_server_addr, _))
            .WillOnce(Return(ack_packet.ToBigEndianVector().size()));


        block_id++;
        
    }
    fin.close();

    // Call method
    std::vector<BYTE> buffer;
    EXPECT_EQ(m_tftp_client->Get(buffer, fname), TFTPClient::Status::kSuccess);
    // Check results
    std::vector<BYTE> expected_data = ReadAllBinaryData(input_fname);
    EXPECT_EQ(buffer, expected_data);
}


///////////////////////////////////////////////////////////////////////////////
/*
    @brief Test of Put method
        when SendRequest failed since WriteDatagram returns zero
*/
TEST_F(TFTPClientTest, PutResults_SendRequestReturnsError_Failed)
{
    // WriteRequest packet
    std::string fname = TFTPClient::GetUploadedDefaultFName();
    std::vector<BYTE> req = CreateRequest(fname, OpCode::WRQ);
  
    EXPECT_CALL(m_mock_socket, WriteDatagram(req, m_server_addr, m_port))
            .WillOnce(Return(0));
    
    // Call method
    std::vector<BYTE> buffer {1, 2, 3};
    EXPECT_EQ(m_tftp_client->Put(buffer, fname), TFTPClient::Status::kSendRequestError);
}

/*
    @brief Test of Put method
        when SendRequest and PutFile successed 
        while sending less than 512 bytes of payload
*/
TEST_F(TFTPClientTest, PutResults_SendDataSmallerThanMaxDataSize_Success)
{
    const std::string input_fname = "test_data/data_500B.bin";
    ASSERT_TRUE(std::filesystem::exists(input_fname));
    size_t data_size = std::filesystem::file_size(input_fname);
    // Check filesize
    ASSERT_GT(data_size, 0);
    ASSERT_LE(data_size, m_max_data_size) << "Filesize of " << input_fname 
            << "is bigger than " << m_max_data_size << " bytes.";
    
    uint16_t block_id = 0;

    // Set expectations
    InSequence s;

    // Send write request
    const std::string uploadFName = TFTPClient::GetUploadedDefaultFName();
    ssize_t WRQ_packet_size = 2 +  uploadFName.size() + 1 + strlen("octet") + 1 ;
    EXPECT_CALL(m_mock_socket, WriteDatagram(_, m_server_addr, m_port)).WillOnce(Return(WRQ_packet_size));
    
    // Read ACK #1
    AckPacket ack_packet_1(block_id); 
    std::vector<BYTE> ack_bytes = ack_packet_1.ToBigEndianVector();
    EXPECT_CALL(m_mock_socket, ReadDatagram(_, _, m_server_addr, _))
            .WillOnce(DoAll(
                SetArgReferee<0>(ack_bytes), 
                Return(ack_bytes.size())
            ));

    // Send data in one packet
    block_id++;
    std::vector data_bytes = ReadAllBinaryData(input_fname);
    DataPacket data_packet (block_id, data_bytes);
    EXPECT_CALL(m_mock_socket, WriteDatagram(data_packet.ToBigEndianVector(), m_server_addr, _))
        .WillOnce(Return(data_packet.ToBigEndianVector().size()));

    // Read ACK #2
    AckPacket ack_packet_2(block_id); 
    ack_bytes = ack_packet_2.ToBigEndianVector();
    EXPECT_CALL(m_mock_socket, ReadDatagram(_, _, m_server_addr, _))
            .WillOnce(DoAll(
                SetArgReferee<0>(ack_bytes), 
                Return(ack_bytes.size())
            ));

    // Call method
    EXPECT_EQ(m_tftp_client->Put(data_bytes, uploadFName), TFTPClient::Status::kSuccess);
}


/*
    @brief Test of Put method
        when SendRequest and PutFile successed 
        while sending more than 512 bytes of payload
*/
TEST_F(TFTPClientTest, PutResults_Send2000Bytes_Success)
{
    const std::string input_fname = "test_data/data_2000B.txt";
    ASSERT_TRUE(std::filesystem::exists(input_fname));
    size_t data_size = std::filesystem::file_size(input_fname);
    // Check filesize
    ASSERT_GT(data_size, m_max_data_size) << "Filesize of " << input_fname 
            << "is smaller than " << m_max_data_size << " bytes.";

    // Set expectations
    InSequence s;
    
    // Send write request
    const std::string uploadFName = TFTPClient::GetUploadedDefaultFName();
    ssize_t WRQ_packet_size = 2 +  uploadFName.size() + 1 + strlen("octet") + 1 ;
    EXPECT_CALL(m_mock_socket, WriteDatagram(_, m_server_addr, m_port)).WillOnce(Return(WRQ_packet_size));
    
    uint16_t block_id = 0;
    // Read initial ACK 
    AckPacket ack_initial(block_id); 
    std::vector<BYTE> ack_initial_bytes = ack_initial.ToBigEndianVector();
    EXPECT_CALL(m_mock_socket, ReadDatagram(_, _, m_server_addr, _))
            .WillOnce(DoAll(
                SetArgReferee<0>(ack_initial_bytes), 
                Return(ack_initial_bytes.size())
            ));

    // Send data in 4 packets
    std::fstream fin(input_fname, std::ios_base::in | std::ios_base::binary);
    ASSERT_TRUE(fin.is_open());
    while(!fin.eof())
    {
        std::vector<BYTE> buffer(m_max_data_size);
        // Read kMaxDataSize_ bytes of data 
        fin.read((char*)&buffer[0], m_max_data_size);
        buffer.resize(fin.gcount()); // Adjust size if fin.gcount() < max_data_size

        block_id++;
        DataPacket data_packet (block_id, buffer);
        // Send data expectation
        EXPECT_CALL(m_mock_socket, WriteDatagram(data_packet.ToBigEndianVector(), m_server_addr, _))
                .WillOnce(Return(data_packet.ToBigEndianVector().size()));

        AckPacket ack_data(block_id); 
        std::vector<BYTE> ack_data_bytes = ack_data.ToBigEndianVector();
        // Read ACK expectation
        EXPECT_CALL(m_mock_socket, ReadDatagram(_, _, m_server_addr, _))
                .WillOnce(DoAll(
                    SetArgReferee<0>(ack_data_bytes), 
                    Return(ack_data_bytes.size())
                ));
        ASSERT_LT(block_id, 5) << "Too many data chunks have been read"; // 2000 bytes will be sent in 4 packets
    }
    fin.close();

    // Call method
    std::vector<BYTE> data_bytes = ReadAllBinaryData(input_fname);
    EXPECT_EQ(m_tftp_client->Put(data_bytes, uploadFName), TFTPClient::Status::kSuccess);
}


int main(int argc, char** argv)
{   
    ::testing::InitGoogleTest(&argc, argv);
    ::testing::InitGoogleMock(&argc, argv);
    return RUN_ALL_TESTS();
}
