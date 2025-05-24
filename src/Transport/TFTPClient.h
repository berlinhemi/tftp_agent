#ifndef TFTPCLIENT_H
#define TFTPCLIENT_H

#include "UDPSocket/UDPSocket.h"
#include "TFTPPacketTypes.h"

<<<<<<< HEAD

=======
>>>>>>> origin/dev
#include <netinet/in.h>

#include <array>
#include <string>


<<<<<<< HEAD

=======
>>>>>>> origin/dev
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
<<<<<<< HEAD
        kReadFileError
    };

    //TFTPClient(const std::string& server_addr, uint16_t port);
    TFTPClient(UdpSocket* udp_sock, const std::string& server_addr, uint16_t port);

    Status Get(const std::string& file_name);
    Status Put(const std::string& file_name);

    std::string ErrorDescription(Status code);
    uint8_t GetHeaderSize();
    uint16_t GetDataSize();
=======
        kReadFileError,
        kSendRequestError
    };

    enum class RequestType
    {
        GET = 0,
        PUT,
        UNKNOWN
    };

    TFTPClient(UdpSocket* udp_sock, const std::string& server_addr, uint16_t port);

    Status Get(std::vector<BYTE>& buffer, const std::string& fname);
    Status Put(const std::vector<BYTE>& data, const std::string& fname);
    
    static uint8_t GetHeaderSize() ;
    static uint16_t GetMaxDataSize();
    static std::string GetDownloadedDefaultFName();
    static std::string GetUploadedDefaultFName();
    std::string ErrorDescription(Status code) const;
>>>>>>> origin/dev

    ~TFTPClient() = default;

private:

    static inline const std::string kDownloadedDefaultFname = "input";
    static inline const std::string kUploadedDefaultFname = "output";
    static const uint8_t kHeaderSize = 4;
    static const uint16_t kDataMaxSize = 512;

    using Result = std::pair<Status, int32_t>;

    Status SendRequest(const std::string& file_name, OpCode opCode);
    Status SendAck(const std::string& host, uint16_t port);
    Status GetData(std::vector<BYTE>& buffer);
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
