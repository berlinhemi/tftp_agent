#include "Agent/Agent.h"
#include "Transport/ITFTPClient.h"
#include "Agent/Executor.h"
#include "Agent/Packer.h"

#include "mock/MockITFTPClient.h"
#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "easylogging++.h"
#include "easylogging++.cc"

using ::testing::_;
using ::testing::Return;
using ::testing::DoAll;
using ::testing::SetArgReferee;
using ::testing::InSequence;
using ::testing::Invoke;
using ::testing::StrEq;

INITIALIZE_EASYLOGGINGPP

/*
    @brief Fixture for Agent tests
*/
class AgentTest : public testing::Test
{
protected:
    std::shared_ptr<MockITFTPClient> m_mockClient;
    std::unique_ptr<Agent> m_agent;
    std::string m_encryptionKey = "test_key_123";
    std::string m_host = "1.1.1.1";
    uint16_t m_port = 69;

    void SetUp() override
    {
        el::Configurations defaul_conf;
        defaul_conf.setToDefault();
        defaul_conf.set(el::Level::Info, el::ConfigurationType::Enabled, "true");
        defaul_conf.set(el::Level::Debug, el::ConfigurationType::Enabled, "false");
        defaul_conf.set(el::Level::Error, el::ConfigurationType::Enabled, "true");
        
        el::Loggers::reconfigureLogger("default", defaul_conf);
        el::Loggers::reconfigureLogger("default", el::ConfigurationType::Format, "[%level] %msg");
        el::Loggers::reconfigureLogger("default", el::ConfigurationType::ToFile, "false");

        m_mockClient = std::make_shared<MockITFTPClient>();
        
        m_agent = std::make_unique<Agent>(m_mockClient.get(), m_encryptionKey);
    }

    void TearDown() override
    {}

    /*
        @brief Helper: create encrypted command
    */
    std::vector<BYTE> EncryptCommand(const std::string& command)
    {
        Packer packer(m_encryptionKey);
        std::vector<BYTE> data(command.begin(), command.end());
        return packer.Pack(data);
    }

    /*
        @brief Helper: decrypt result
    */
    std::string DecryptResult(const std::vector<BYTE>& encrypted_data)
    {
        Packer packer(m_encryptionKey);
        std::vector<BYTE> unpacked = packer.Unpack(encrypted_data);
        return std::string(unpacked.begin(), unpacked.end());
    }
};

/*
    @brief Test DoIteration when GetCommand fails
*/
TEST_F(AgentTest, DoIteration_GetCommandFails_LogsErrorAndReturns)
{
    // Expect: Get() call returns error
    EXPECT_CALL(*m_mockClient, Get(_, _))
        .WillOnce(Return(ITFTPClient::Status::kReadError));
    
    // Expect: Put() should NOT be called
    EXPECT_CALL(*m_mockClient, Put(_, _))
        .Times(0);
    
    // Call method
    EXPECT_NO_THROW(m_agent->DoIteration());
}

/*
    @brief Test DoIteration when command is empty
*/
TEST_F(AgentTest, DoIteration_EmptyCommand_LogsErrorAndReturns)
{
    // Create empty command (after unpacking)                                       
    std::vector<BYTE> empty_command = EncryptCommand("");
    
    // Expect: Get() returns success with empty data
    EXPECT_CALL(*m_mockClient, Get(_, _))
        .WillOnce(DoAll(
            SetArgReferee<0>(empty_command),
            Return(ITFTPClient::Status::kSuccess)
        ));
    
    // Expect: Put() should NOT be called
    EXPECT_CALL(*m_mockClient, Put(_, _))
        .Times(0);
    
    // Call method
    EXPECT_NO_THROW(m_agent->DoIteration());
}

/*
    @brief Test DoIteration successful flow
*/
TEST_F(AgentTest, DoIteration_SuccessfulExecution_ReturnsSuccess)
{
    // Prepare test data
    std::string command = "echo Hello World";
    
    // Encrypt command (simulate what server would send)
    std::vector<BYTE> encrypted_command = EncryptCommand(command);
    
    // Prepare expected result
    CommandResult expected_result;
    expected_result.exitCode = ExecStatus::Success;
    expected_result.std_out = "Hello World\n";
    expected_result.std_err = "";
    
    // InSequence for ordered expectations
    InSequence seq;
    
    // Expectation 1: Get() called to download command
    EXPECT_CALL(*m_mockClient, Get(_, _))
        .WillOnce(DoAll(
            SetArgReferee<0>(encrypted_command),
            Return(ITFTPClient::Status::kSuccess)
        ));
    
    // Expectation 2: Put() called to upload result
    EXPECT_CALL(*m_mockClient, Put(_, _))
        .WillOnce(Return(ITFTPClient::Status::kSuccess));
    
    // Call method
    EXPECT_NO_THROW(m_agent->DoIteration());
}

/*
    @brief Test DoIteration when SendResult fails
*/
TEST_F(AgentTest, DoIteration_SendResultFails_LogsError)
{
    // Prepare test data
    std::string command = "echo Test";
    std::vector<BYTE> encrypted_command = EncryptCommand(command);
    
    // InSequence for ordered expectations
    InSequence seq;
    
    // Expectation 1: Get() succeeds
    EXPECT_CALL(*m_mockClient, Get(_, _))
        .WillOnce(DoAll(
            SetArgReferee<0>(encrypted_command),
            Return(ITFTPClient::Status::kSuccess)
        ));
    
    // Expectation 2: Put() fails
    EXPECT_CALL(*m_mockClient, Put(_, _))
        .WillOnce(Return(ITFTPClient::Status::kWriteError));
    
    // Call method - should not throw even though Put failed
    EXPECT_NO_THROW(m_agent->DoIteration());
}


int main(int argc, char** argv)
{   
    ::testing::InitGoogleTest(&argc, argv);
    ::testing::InitGoogleMock(&argc, argv);
    return RUN_ALL_TESTS();
}                                                                                                                                                                                                                                                            