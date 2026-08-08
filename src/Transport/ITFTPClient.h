#ifndef ITFTPCLIENT_H
#define ITFTPCLIENT_H

#include "UDPSocket/UDPSocket.h"
#include "TFTPPacketTypes.h"


#include <string>


typedef unsigned char BYTE;


class ITFTPClient
{
public:
    virtual ~ITFTPClient() = default;


    enum class Status {
        kSuccess = 0,           ///< Operation completed successfully
        kInvalidSocket,         ///< Socket is not properly initialized
        kWriteError,            ///< Error writing to socket
        kReadError,             ///< Error reading from socket
        kUnexpectedPacketReceived, ///< Received packet with unexpected opcode
        kEmptyFilename,         ///< Filename string is empty
        kOpenFileError,         ///< Cannot open local file
        kWriteFileError,        ///< Error writing to local file
        kReadFileError,         ///< Error reading from local file
        kSendRequestError       ///< Failed to send RRQ/WRQ request
    };

    enum class RequestType{
        GET = 0,     ///< Read request (RRQ)
        PUT,         ///< Write request (WRQ)
        UNKNOWN      
    };
    virtual Status Get(std::vector<BYTE>& buffer, const std::string& fname) = 0;
    virtual Status Put(const std::vector<BYTE>& data, const std::string& fname) = 0;
   
};

#endif // ITFTPCLIENT_H
