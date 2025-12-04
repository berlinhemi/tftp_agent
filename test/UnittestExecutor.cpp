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

   
};


/*
    @brief Test of Execute method
            when command is empty
*/
TEST_F(AgentExecutorTest, Execute_EmptyCommand_ExitSuccessNoOutput)
{
    std::string command = "";
    std::optional<CommandResult>  result = Executor::Execute(command);
    ASSERT_TRUE(result.has_value());
   
    std::cout << result.value().output << std::endl;
    std::cout << result.value().error << std::endl;
    EXPECT_EQ(result.value().output, std::string());
    EXPECT_EQ(result.value().error, std::string());
    EXPECT_EQ(result.value().exitCode, EXIT_SUCCESS);

}



int main(int argc, char** argv)
{   
    ::testing::InitGoogleTest(&argc, argv);
    ::testing::InitGoogleMock(&argc, argv);
    return RUN_ALL_TESTS();
}