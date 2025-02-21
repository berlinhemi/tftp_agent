#ifndef TFTPCLIENT_H
#define TFTPCLIENT_H

#include "UDPSocket/UDPSocket.h"
#include "TFTPPacket.h"

#include <netinet/in.h>

#include <array>
#include <string>

typedef unsigned char BYTE;

class TFTPClient
{
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

    //TFTPClient(const std::string& server_addr, uint16_t port);
    TFTPClient(UdpSocket* udp_sock, const std::string& server_addr, uint16_t port);

    Status Get(const std::string& file_name);
    Status Put(const std::string& file_name);

    std::string ErrorDescription(Status code);
    uint8_t GetHeaderSize();
    uint16_t GetDataSize();

    ~TFTPClient();

private:
    static const uint8_t kHeaderSize = 4;
    static const uint16_t kDataSize = 512;

    using Result = std::pair<Status, int32_t>;
    //using Buffer = std::array<char, kHeaderSize + kDataSize>;

    Result SendRequest(const std::string& file_name, OpCode code);
    Result SendAck(const std::string& host, uint16_t port);
    Result Read();
    Result GetFile(std::fstream& file);
    Result PutFile(std::fstream& file);

    UdpSocket* socket_;
    std::string remote_addr_;
    uint16_t initial_port_;
    uint16_t remote_port_; //tftp server changes port after first connection
    //todo: delete buffer_ ?
    std::vector<BYTE> buffer_;
    //std::array<BYTE, kHeaderSize + kDataSize> buffer_;
    uint16_t received_block_id_;
    Status status_;
};

#endif // TFTPCLIENT_H
