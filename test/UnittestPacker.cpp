#include "Agent/Packer.h"

#include <cstring>
#include <fstream>
#include <filesystem>
#include <memory>
#include <random>
#include <chrono>

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

class AgentPackerTest : public testing::Test
{
protected:
    void SetUp() override
    {
        // Configure logging similarly to Executor tests
        el::Configurations defaul_conf;
        defaul_conf.setToDefault();
        defaul_conf.set(el::Level::Info, el::ConfigurationType::Enabled, "true");
        defaul_conf.set(el::Level::Debug, el::ConfigurationType::Enabled, "false");
        defaul_conf.set(el::Level::Error, el::ConfigurationType::Enabled, "true");
        
        el::Loggers::reconfigureLogger("default", defaul_conf);
        el::Loggers::reconfigureLogger("default", el::ConfigurationType::Format, "[%level] %msg");
        el::Loggers::reconfigureLogger("default", el::ConfigurationType::ToFile, "false");
        
        // Initialize random generator
        m_rng.seed(std::chrono::steady_clock::now().time_since_epoch().count());
    }
    
    void TearDown() override
    {
        // Cleanup if needed
    }
    
    // Helper: Generate random data
    std::vector<BYTE> GenerateRandomData(size_t size)
    {
        std::vector<BYTE> data(size);
        std::uniform_int_distribution<int> dist(0, 255);
        
        for (size_t i = 0; i < size; ++i) {
            data[i] = static_cast<BYTE>(dist(m_rng));
        }
        return data;
    }
    
    // Helper: Generate repetitive data (good for compression testing)
    std::vector<BYTE> GenerateRepetitiveData(size_t size, BYTE pattern = 'A')
    {
        return std::vector<BYTE>(size, pattern);
    }
    
    // Helper: Convert string to byte vector
    std::vector<BYTE> StringToBytes(const std::string& str)
    {
        return std::vector<BYTE>(str.begin(), str.end());
    }
    
    // Helper: Convert byte vector to string
    std::string BytesToString(const std::vector<BYTE>& bytes)
    {
        return std::string(bytes.begin(), bytes.end());
    }
    
    // Helper: Check if pack/unpack roundtrip works
    bool VerifyRoundTrip(Packer& packer, const std::vector<BYTE>& original)
    {
        auto packed = packer.Pack(original);
        if (packed.empty() && !original.empty()) {
            return false;
        }
        
        auto unpacked = packer.Unpack(packed);
        return original == unpacked;
    }
    
    // Helper: Check if string contains substring (case insensitive)
    bool CaseInsensitiveContains(const std::string& str, const std::string& substr)
    {
        auto it = std::search(
            str.begin(), str.end(),
            substr.begin(), substr.end(),
            [](char ch1, char ch2) {
                return std::tolower(ch1) == std::tolower(ch2);
            }
        );
        return it != str.end();
    }
    
    // Test constants
    static const int kTestTimeoutSec = 10;
    static const size_t kSmallDataSize = 1024;          // 1 KB
    static const size_t kMediumDataSize = 1024 * 1024;  // 1 MB
    static const size_t kLargeDataSize = 5 * 1024 * 1024; // 5 MB
    static const std::string kDefaultKey;
    
private:
    std::mt19937 m_rng;
};

// Define static member
const std::string AgentPackerTest::kDefaultKey = "test_key_123";


/*
    @brief Test of Pack method
            when buffer is empty
*/
TEST_F(AgentPackerTest, Pack_EmptyBuffer_ReturnsEmpty)
{
    Packer packer(kDefaultKey);
    std::vector<BYTE> empty;
    
    auto result = packer.Pack(empty);
    
    EXPECT_TRUE(result.empty());
}

/*
    @brief Test of Unpack method
            when buffer is empty
*/
TEST_F(AgentPackerTest, Unpack_EmptyBuffer_ReturnsEmpty)
{
    Packer packer(kDefaultKey);
    std::vector<BYTE> empty;
    
    auto result = packer.Unpack(empty);
    
    EXPECT_TRUE(result.empty());
}

/*
    @brief Test of Pack/Unpack roundtrip
            with valid data
*/
TEST_F(AgentPackerTest, PackUnpack_ValidData_RoundtripSuccess)
{
    Packer packer(kDefaultKey);
    std::string original = "This is a test message.";
    auto data = StringToBytes(original);
    
    auto packed = packer.Pack(data);
    EXPECT_FALSE(packed.empty());
    
    auto unpacked = packer.Unpack(packed);
    std::string result = BytesToString(unpacked);
    
    EXPECT_EQ(original, result);
}

/*
    @brief Test of Pack/Unpack roundtrip
            with binary data
*/
TEST_F(AgentPackerTest, PackUnpack_BinaryData_RoundtripSuccess)
{
    Packer packer(kDefaultKey);
    auto data = GenerateRandomData(kSmallDataSize);
    
    EXPECT_TRUE(VerifyRoundTrip(packer, data));
}

/*
    @brief Test of Pack
            when data exceeds MAX_DATA_SIZE
*/
TEST_F(AgentPackerTest, Pack_ExceedsMaxSize_ReturnsEmpty)
{
    Packer packer(kDefaultKey);
    auto data = GenerateRandomData(Packer::kMaxDataSizeBytes + 1);
    
    auto result = packer.Pack(data);
    
    EXPECT_TRUE(result.empty());
}

/*
    @brief Test of Pack/Unpack
            with empty key
*/
TEST_F(AgentPackerTest, Pack_EmptyKey_Fails)
{
    Packer packer(""); // key is empty
    std::string original = "Test message";
    auto data = StringToBytes(original);
    
    auto packed = packer.Pack(data);
    EXPECT_TRUE(packed.empty());
}


/*
    @brief Test of SetKey
            with valid and invalid keys
*/
TEST_F(AgentPackerTest, SetKey_ValidAndInvalid_WorksCorrectly)
{
    Packer packer("initial_key");
    std::string original = "Secret data";
    auto data = StringToBytes(original);
    
    // Pack with initial key
    auto packed_initial = packer.Pack(data);
    
    // Change to empty key (should fail)
    bool result = packer.SetKey("");
    EXPECT_FALSE(result);
    
    // Pack with same key (should still work)
    auto packed_same = packer.Pack(data);
    EXPECT_EQ(packed_initial, packed_same);
    
    // Change to new valid key
    result = packer.SetKey("new_valid_key");
    EXPECT_TRUE(result);
    
    // Pack with new key (should be different)
    auto packed_new = packer.Pack(data);
    EXPECT_NE(packed_initial, packed_new);
    
    // Unpack with new key should work
    auto unpacked = packer.Unpack(packed_new);
    EXPECT_EQ(original, BytesToString(unpacked));
}

/*
    @brief Test of compression
            repetitive data should compress well
*/
TEST_F(AgentPackerTest, Compression_RepetitiveData_ReducesSize)
{
    Packer packer(kDefaultKey);
    auto data = GenerateRepetitiveData(10000, 'A');
    
    auto packed = packer.Pack(data);
    
    // Compressed + encrypted data should be smaller than original
    EXPECT_LT(packed.size(), data.size());
}

/*
    @brief Test of Unpack
            with corrupted data
*/
TEST_F(AgentPackerTest, Unpack_CorruptedData_ReturnsEmpty)
{
    Packer packer(kDefaultKey);
    auto data = StringToBytes("Test data");
    
    auto packed = packer.Pack(data);
    ASSERT_FALSE(packed.empty());
    
    // Corrupt the data
    packed[packed.size() / 2] ^= 0xFF;
    
    auto unpacked = packer.Unpack(packed);
    EXPECT_TRUE(unpacked.empty());
}

/*
    @brief Test of Pack/Unpack
            with different keys
*/
TEST_F(AgentPackerTest, DifferentKeys_CannotDecrypt)
{
    Packer packer1("key1");
    Packer packer2("key2");
    
    auto data = StringToBytes("Secret message");
    
    auto packed = packer1.Pack(data);
    auto unpacked = packer2.Unpack(packed);
    
    // Should fail to decrypt correctly
    EXPECT_NE(data, unpacked);
    // Should either be empty or different data
    EXPECT_TRUE(unpacked.empty() || unpacked != data);
}

int main(int argc, char** argv)
{   
    ::testing::InitGoogleTest(&argc, argv);
    ::testing::InitGoogleMock(&argc, argv);
    return RUN_ALL_TESTS();
}