#include "Agent/Executor.h"

#include <cstring>
#include <fstream>
#include <filesystem>
#include <memory>
#include <sys/resource.h>  
#include <fcntl.h> 

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "Base64.h"

#include "easylogging++.h"
#include "easylogging++.cc"

using ::testing::_;
using ::testing::SetArgReferee;
using ::testing::DoAll;
using ::testing::Return;
using ::testing::InSequence;
using ::testing::Expectation;
using ::testing::Pointee;
using ::testing::Invoke;

// Define it only once in project
INITIALIZE_EASYLOGGINGPP 


class AgentExecutorTest : public testing::Test
{

protected:
  
    void SetUp()
    {
        el::Configurations defaul_conf;
        defaul_conf.setToDefault();
        defaul_conf.set(el::Level::Info, el::ConfigurationType::Enabled, "true");
        defaul_conf.set(el::Level::Debug, el::ConfigurationType::Enabled, "false");
        
        el::Loggers::reconfigureLogger("default", defaul_conf);
        el::Loggers::reconfigureLogger("default", el::ConfigurationType::Format, "[%level] %msg");
        el::Loggers::reconfigureLogger("default", el::ConfigurationType::ToFile, "false");
     
    }
    void TearDown() {}

    // Helper
    bool CaseInsensitiveContains(const std::string& str, const std::string& substr) {
        auto it = std::search(
            str.begin(), str.end(),
            substr.begin(), substr.end(),
            [](char ch1, char ch2) {
                return std::toupper(ch1) == std::toupper(ch2);
            }
        );
        return it != str.end();
    }

    static const int kTestTimeoutSec = 10;
};


/*
    @brief Test of Execute method
            when command is empty
*/
TEST_F(AgentExecutorTest, Execute_EmptyCommand_SuccessNoOutput)
{
    // no command
    std::string command = "";
    CommandResult result = Executor::Execute(command, kTestTimeoutSec);

    EXPECT_EQ(result.output, std::string());
    EXPECT_EQ(result.error, std::string());
    EXPECT_EQ(result.exitCode, ExecStatus::Success);
}

/*
    @brief Test of Execute method
            when command is non-existing script/binary
*/
TEST_F(AgentExecutorTest, Execute_CommandIsNonExisting_ErrorNotFound)
{
    std::string command = "invalid_name";
    CommandResult result = Executor::Execute(command, kTestTimeoutSec);

    EXPECT_EQ(result.output, std::string());
    EXPECT_TRUE(CaseInsensitiveContains(result.error, "not found"));
    EXPECT_EQ(result.exitCode, ExecStatus::CommandNotFound);
}


/*
    @brief Test of Execute method
        when pipe() failed since limit of file descriptors is 0
*/
TEST_F(AgentExecutorTest, Execute_FileLimitExceed_PipeError) {
    // Save current limits
    struct rlimit old_limit;
    getrlimit(RLIMIT_NOFILE, &old_limit);
    
    // Set new limit, but do not change hard limit 
    // so we could restore it as any user
    struct rlimit new_limit = {0, old_limit.rlim_max};  
    
    if (setrlimit(RLIMIT_NOFILE, &new_limit) == 0) {
        CommandResult result = Executor::Execute("echo test", kTestTimeoutSec);
        
        EXPECT_EQ(result.exitCode, ExecStatus::PipeFailed);
        EXPECT_TRUE(result.output.empty());
        EXPECT_TRUE(result.error.empty());
                
        if (setrlimit(RLIMIT_NOFILE, &old_limit) == -1) {
            std::cout << "restore failed: " << strerror(errno) << std::endl;
        }
    } else {
        // Skip if limits cant be set
        GTEST_SKIP() << "setrlimit failed, error: " << strerror(errno);
    }
}


/*
    @brief Test of Execute method
        when command is valid but parameter is not valid
*/
TEST_F(AgentExecutorTest, Execute_lsCommandInvalidParameter_Error)
{
    struct rlimit current;
    getrlimit(RLIMIT_NOFILE, &current);
    // std::cout << "Current NOFILE limits: soft=" << current.rlim_cur 
    //           << ", hard=" << current.rlim_max << std::endl;

    std::string command = "ls -l /path/not/exist";
    CommandResult result = Executor::Execute(command, kTestTimeoutSec);

    EXPECT_EQ(result.output, std::string());
    // std::cout << result.output << std::endl;
    // std::cout << result.error << std::endl;
    // std::cout << (int)result.exitCode << std::endl;
    EXPECT_TRUE(CaseInsensitiveContains(result.error, "no such file"));
    EXPECT_NE(result.exitCode, ExecStatus::Success);
}

/*
    @brief Test of Execute method
        when command is valid and some stdout expected
*/
TEST_F(AgentExecutorTest, Execute_lsCommandValidParameter_Success)
{
    std::string command = "ls -l /";
    CommandResult result = Executor::Execute(command, kTestTimeoutSec);

    EXPECT_TRUE(CaseInsensitiveContains(result.output, "etc"));
    EXPECT_TRUE(CaseInsensitiveContains(result.output, "bin"));
    EXPECT_TRUE(CaseInsensitiveContains(result.output, "boot"));
    EXPECT_EQ(result.exitCode, ExecStatus::Success);
}

/*
    @brief Test of Execute method
        when command is valid and some stdout expected
*/
TEST_F(AgentExecutorTest, Execute_touchCommand_SuccessNoOutput)
{
    std::string tmp_file = "/tmp/test.file";
    std::string command = "touch ";
    command += tmp_file;
    CommandResult result = Executor::Execute(command, kTestTimeoutSec);

    EXPECT_EQ(result.output, std::string());
    EXPECT_EQ(result.error, std::string());
    EXPECT_TRUE(std::filesystem::exists(tmp_file));
    EXPECT_TRUE(std::filesystem::remove(tmp_file));
    EXPECT_EQ(result.exitCode, ExecStatus::Success);
}


/*
    @brief Test of Execute method
        when command contains base64 decoding
        and saving results to file
*/
TEST_F(AgentExecutorTest, Execute_SaveToFileB64Data_SuccessNoOutput)
{
    std::string test_message = "test message for encoding";
    Base64 encoder(test_message, Base64::TextEncode);
    
    std::string tmp_file = "/tmp/test.file";
    std::string command = "echo \"";
    command += encoder.encode();
    command += "\" | base64 -d > ";
    command += tmp_file;
    CommandResult result = Executor::Execute(command, kTestTimeoutSec);
    // std::cout << command << std::endl;

    //std::cout << test.encode();
    EXPECT_EQ(result.output, std::string());
    EXPECT_EQ(result.error, std::string());
    EXPECT_TRUE(std::filesystem::exists(tmp_file));
    std::ifstream ifs(tmp_file);
    std::string decoded_message;
    if (ifs) {
        decoded_message.assign(std::istreambuf_iterator<char>(ifs),
                           std::istreambuf_iterator<char>());
        ifs.close();
    }   

    EXPECT_EQ(test_message, decoded_message);
    EXPECT_TRUE(std::filesystem::remove(tmp_file));
    EXPECT_EQ(result.exitCode, ExecStatus::Success);
}


/*
    @brief Test of Execute method
        when ping is limited to 2 packets
*/
TEST_F(AgentExecutorTest, Execute_ShortPing_Success)
{
    std::string host = "8.8.8.8";
    std::string command = "ping -c 2 " + host;
       
    CommandResult result = Executor::Execute(command, kTestTimeoutSec);
    
    EXPECT_TRUE(CaseInsensitiveContains(result.output, "ping"));
    EXPECT_TRUE(CaseInsensitiveContains(result.output, std::string("bytes from ") + host));
    EXPECT_TRUE(result.error.empty());
    
    EXPECT_EQ(result.exitCode, ExecStatus::Success);
}


int main(int argc, char** argv)
{   
    ::testing::InitGoogleTest(&argc, argv);
    ::testing::InitGoogleMock(&argc, argv);
    return RUN_ALL_TESTS();
}