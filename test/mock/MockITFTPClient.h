#ifndef MOCKITFTPCLIENT_H
#define MOCKITFTPCLIENT_H

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <cstring>
#include <fstream>
#include <filesystem>

#include "Transport/ITFTPClient.h"

class MockITFTPClient: public ITFTPClient
{
public:

    MOCK_METHOD(Status, 
        Get, (std::vector<BYTE>& buffer, const std::string& fname), 
        (override));

    MOCK_METHOD(Status,  
        Put, (const std::vector<BYTE>& data, const std::string& fname),
         (override));
};

#endif // MOCKITFTPCLIENT_H