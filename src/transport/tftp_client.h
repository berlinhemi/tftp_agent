#ifndef TFTPCLIENT_H
#define TFTPCLIENT_H

#include "udp_socket.h"
#include <array>
#include <string>

class TFTPClient
{
    enum class OpCode {
        RRQ = 1, WRQ, DATA, ACK, ERR, OACK
    };

public:
    enum class Status {
        kSuccess = 0,
        kInvalidSocket,
        kWriteError,
        kReadError,
        kUnexpectedPacketReceived,
        kEmptyFilename,
        kOpenFileError,
        kWriteFileError,
        kReadFileError
    };

    TFTPClient(const std::string &server_addr, uint16_t port);

    Status Get(const std::string &file_name);
    Status Put(const std::string &file_name);

    std::string ErrorDescription(Status code);

private:
    static const uint8_t kHeaderSize = 4;
    static const uint16_t kDataSize = 512;

    using Result = std::pair<Status, int32_t>;
    using Buffer = std::array<char, kHeaderSize + kDataSize>;

    Result SendRequest(const std::string &file_name, OpCode code);
    Result SendAck(const char *host, uint16_t port);
    Result Read();
    Result GetFile(std::fstream &file);
    Result PutFile(std::fstream &file);

    UdpSocket socket_;
    std::string remote_addr_;
    uint16_t port_;
    uint16_t remote_port_;
    Buffer buffer_;
    uint16_t received_block_;
    Status status_;
};

#endif // TFTPCLIENT_H
