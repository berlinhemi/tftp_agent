#ifndef TFTPCLIENT_H
#define TFTPCLIENT_H

#include "UDPSocket/UDPSocket.h"
#include "TFTPPacketTypes.h"


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
        kReadFileError,
        kSendRequestError
    };

    TFTPClient(UdpSocket* udp_sock, const std::string& server_addr, uint16_t port);

    Status GetCommand(std::vector<BYTE>& buffer);
    Status Put(const std::string& file_name);

    std::string ErrorDescription(Status code);
    uint8_t GetHeaderSize();
    uint16_t GetDataSize();

    ~TFTPClient() = default;

private:
    static inline const std::string kCmdFname = "command";
    static const uint8_t kHeaderSize = 4;
    static const uint16_t kDataSize = 512;

    using Result = std::pair<Status, int32_t>;

    Status SendRequest(const std::string& file_name, OpCode code);
    Status SendAck(const std::string& host, uint16_t port);
    Result Read();
    Status GetData(std::vector<BYTE>& command);
    Result PutFile(std::fstream& file);

    UdpSocket* m_socket;
    std::string m_remote_addr;
    uint16_t m_initial_port;
    uint16_t m_remote_port; //tftp server changes port after first connection
    //todo: delete buffer_ ?
    std::vector<BYTE> m_buffer;
    //std::array<BYTE, kHeaderSize + kDataSize> buffer_;
    uint16_t m_received_block_id;
    Status m_status;
};

#endif // TFTPCLIENT_H
