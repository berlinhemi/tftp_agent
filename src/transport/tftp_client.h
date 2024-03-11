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
        Success = 0,
        InvalidSocket,
        WriteError,
        ReadError,
        UnexpectedPacketReceived,
        EmptyFilename,
        OpenFileError,
        WriteFileError,
        ReadFileError
    };

    TFTPClient(const std::string &serverAddress, uint16_t port);

    Status Get(const std::string &fileName);
    Status Put(const std::string &fileName);

    std::string ErrorDescription(Status code);

private:
    static constexpr uint8_t m_headerSize = 4;
    static constexpr uint16_t m_dataSize = 512;

    using Result = std::pair<Status, int32_t>;
    using Buffer = std::array<char, m_headerSize + m_dataSize>;

    Result SendRequest(const std::string &fileName, OpCode code);
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
