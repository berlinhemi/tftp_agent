#include "Agent/Executor.h"

#include <cstring>
#include <fstream>
#include <filesystem>
#include <memory>
#include <sys/resource.h>  
#include <fcntl.h> 

#include <gtest/gtest.h>
#include <gmock/gmock.h>

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
};


/*
    @brief Test of Execute method
            when command is empty
*/
TEST_F(AgentExecutorTest, Execute_EmptyCommand_SuccessNoOutput)
{
    // no command
    std::string command = "";
    CommandResult result = Executor::Execute(command);

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
    CommandResult result = Executor::Execute(command);

    EXPECT_EQ(result.output, std::string());
    EXPECT_TRUE(CaseInsensitiveContains(result.error, "not found"));
    EXPECT_EQ(result.exitCode, ExecStatus::CommandNotFound);
}


/*
    @brief Test of Execute method
        when pipi() failed since limit of file descriptors is 0
*/
TEST_F(AgentExecutorTest, Execute_FileLimitExceed_PipeError) {
    // Save current limits
    struct rlimit old_limit;
    getrlimit(RLIMIT_NOFILE, &old_limit);
    
    // Try to set limit 0
    struct rlimit new_limit = {0, 0};
    if (setrlimit(RLIMIT_NOFILE, &new_limit) == 0) {
        CommandResult result = Executor::Execute("echo test");
        
        EXPECT_EQ(result.exitCode, ExecStatus::PipeFailed);
        EXPECT_TRUE(result.output.empty());
        EXPECT_TRUE(result.error.empty());
        
        // Restore limits
        setrlimit(RLIMIT_NOFILE, &old_limit);
    } else {
        // Skip if limits cant be set to zero
        GTEST_SKIP() << "Test requires ability to set rlimit (run as root?)";
    }
}

/*
    @brief Test of Execute method
        when command is valid but parameter is not valid
*/
TEST_F(AgentExecutorTest, Execute_lsCommandInvalidParameter_Error)
{
    std::string command = "ls -l /path/not/exist";
    CommandResult result = Executor::Execute(command);

    EXPECT_EQ(result.output, std::string());
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
    CommandResult result = Executor::Execute(command);

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
    CommandResult result = Executor::Execute(command);

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
TEST_F(AgentExecutorTest, Execute_decodeB64AndSaveToFile_SuccessNoOutput)
{
    // std::string tmp_file = "/tmp/test.file";
    // std::string command = "touch ";
    // command += tmp_file;
    // CommandResult result = Executor::Execute(command);

    // EXPECT_EQ(result.output, std::string());
    // EXPECT_EQ(result.error, std::string());
    // EXPECT_TRUE(std::filesystem::exists(tmp_file));
    // EXPECT_TRUE(std::filesystem::remove(tmp_file));
    // EXPECT_EQ(result.exitCode, ExecStatus::Success);
}



int main(int argc, char** argv)
{   
    ::testing::InitGoogleTest(&argc, argv);
    ::testing::InitGoogleMock(&argc, argv);
    return RUN_ALL_TESTS();
}