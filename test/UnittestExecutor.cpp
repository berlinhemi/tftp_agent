#include "Agent/Executor.h"

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
TEST_F(AgentExecutorTest, Execute_EmptyCommand_ExitSuccessNoOutput)
{
    // no command
    std::string command = "";
    CommandResult result = Executor::Execute(command);
   
    std::cout << result.output << std::endl;
    std::cout << result.error << std::endl;
    EXPECT_EQ(result.output, std::string());
    EXPECT_EQ(result.error, std::string());
    EXPECT_EQ(result.exitCode, ExecStatus::Success);
}

/*
    @brief Test of Execute method
            when command is non-existing script/binary
*/
TEST_F(AgentExecutorTest, Execute_CommandIsNonExisting_NotFound)
{
    std::string command = "invalid_name";
    CommandResult result = Executor::Execute(command);
   
    std::cout << result.output << std::endl;
    std::cout << result.error << std::endl;
    EXPECT_EQ(result.output, std::string());
    EXPECT_TRUE(CaseInsensitiveContains(result.error, "not found"));
    EXPECT_EQ(result.exitCode, ExecStatus::CommandNotFound);
}



int main(int argc, char** argv)
{   
    ::testing::InitGoogleTest(&argc, argv);
    ::testing::InitGoogleMock(&argc, argv);
    return RUN_ALL_TESTS();
}