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
    Status PutResults(const std::vector<BYTE>& data);
    
    static uint8_t GetHeaderSize() ;
    //static uint16_t GetDataSize() ;
    static uint16_t GetMaxDataSize();
    static std::string GetCommandFName();
    static std::string GetResultFName();
    std::string ErrorDescription(Status code) const;

    ~TFTPClient() = default;

private:
    static inline const std::string kCmdFname = "command";
    static inline const std::string kResultFname = "result";
    static const uint8_t kHeaderSize = 4;
    static const uint16_t kDataMaxSize = 512;

    using Result = std::pair<Status, int32_t>;

    Status SendRequest(const std::string& file_name, OpCode opCode);
    Status SendAck(const std::string& host, uint16_t port);
    Status GetData(std::vector<BYTE>& command);
    Status PutData(const std::vector<BYTE>& data);
    Status Read(std::vector<BYTE>& buffer);

    UdpSocket* m_socket;
    std::string m_remote_addr;
    uint16_t m_initial_port;
    // Note: tftp server changes port durind data exchange
    uint16_t m_remote_port; 
    uint16_t m_received_block_id;
};

#endif // TFTPCLIENT_H
