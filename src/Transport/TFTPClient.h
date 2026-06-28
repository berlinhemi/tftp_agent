#ifndef TFTPCLIENT_H
#define TFTPCLIENT_H

#include "ITFTPClient.h"
#include "UDPSocket/UDPSocket.h"
#include "TFTPPacketTypes.h"

#include <netinet/in.h>

#include <array>
#include <string>


typedef unsigned char BYTE;

/**
 * @brief TFTP (Trivial File Transfer Protocol) client implementation.
 * 
 * Implements TFTP protocol as defined in RFC 1350. Supports both file upload (PUT)
 * and download (GET) operations with automatic handling of block numbers and 
 * acknowledgments.
 * 
 * @details Key features:
 * - Supports octet transfer mode only
 * - Automatic handling of block numbering and acknowledgments
 * - Configurable timeout through underlying UDP socket (3 seconds)
 * - Generates unique filenames for uploads by default
 * 
 * @note This client does not implement TFTP options extension (RFC 2347-2349)
 * @note The server may change its port during data transfer - client handles this automatically
 * 
 * @see UdpSocket For underlying UDP transport
 * @see https://tools.ietf.org/html/rfc1350 TFTP Protocol Specification
 */
class TFTPClient : public ITFTPClient
{
public:
     
   
    /**
     * @brief Main constructor with creation its own UdpSocket.
     * 
     * @param server_addr TFTP server IP address in dotted-decimal format
     * @param port TFTP server port (typically 69 for TFTP)
     * 
     * @throws std::runtime_error if socket initialization fails
     */
    TFTPClient(const std::string& server_addr, uint16_t port);
    
    /**
     * @brief Constructor with dependency injection (for testing)
     * 
     * @param udp_sock Pointer to initialized UDP socket (must remain valid for client lifetime)
     * @param server_addr TFTP server IP address in dotted-decimal format
     * @param port TFTP server port (typically 69 for TFTP)
     * 
     * @throws std::runtime_error if socket is null or not initialized
     * 
     * @warning Caller is responsible for socket lifetime.
     *          Socket must outlive the TFTPClient instance.
     */
    TFTPClient(UdpSocket* udp_sock, const std::string& server_addr, uint16_t port);

    /**
     * @brief Downloads a file from TFTP server.
     * 
     * Sends RRQ (Read Request) to the server and receives file data in chunks.
     * Automatically sends ACK for each received data block.
     * 
     * @param buffer [out] Vector to store downloaded file data
     * @param fname Remote filename to download from server
     * 
     * @return Status code:
     *         - Status::kSuccess on successful download
     *         - Status::kEmptyFilename if fname is empty
     *         - Status::kSendRequestError if RRQ sending failed
     *         - Status::kReadError if data reception failed
     *         - Status::kUnexpectedPacketReceived if protocol violation occurs
     * 
     * @note Function blocks until entire file is received or error occurs
     * @note Received data is appended to buffer (buffer is cleared before download)
     * 
     * @see Put() For file upload
     * @see GetMaxDataSize() Maximum data per packet
     */
    Status Get(std::vector<BYTE>& buffer, const std::string& fname) override;

     /**
     * @brief Uploads a file to TFTP server.
     * 
     * Sends WRQ (Write Request) to the server and transmits data in chunks.
     * Waits for ACK after each data block before sending next chunk.
     * 
     * @param data Vector containing file data to upload
     * @param fname Remote filename to create on server
     * 
     * @return Status code:
     *         - Status::kSuccess on successful upload
     *         - Status::kEmptyFilename if fname is empty
     *         - Status::kSendRequestError if WRQ sending failed
     *         - Status::kWriteError if data transmission failed
     *         - Status::kReadError if ACK reception failed
     * 
     * @note Function blocks until entire file is uploaded or error occurs
     * @note Data is sent in chunks of kDataMaxSize bytes (last chunk may be smaller)
     * 
     * @see Get() For file download
     * @see GetMaxDataSize() Maximum data per packet
     */
    Status Put(const std::vector<BYTE>& data, const std::string& fname) override;
    
    /**
     * @brief Returns default filename for downloaded files.
     * 
     * Used when no custom filename is specified for download operations.
     * 
     * @return Default filename string ("input")
     */
    static std::string GetDownloadedDefaultFName();
    
    /**
     * @brief Generates a unique filename for uploaded files.
     * 
     * Creates timestamp-based unique filenames for uploads to avoid conflicts.
     * Format: "output_DDMMYY_HHMM_<counter>"
     * 
     * @return Unique filename string
     * 
     * @note Counter increments on each call within the same process
     * @example "output_150324_1430_1", "output_150324_1430_2"
     */
    static std::string GetUploadedUniqueFName();

    static uint8_t GetHeaderSize() ;
    static uint16_t GetMaxDataSize();
    static std::string ErrorDescription(Status code);

    ~TFTPClient();

private:
    
    using Result = std::pair<Status, int32_t>;

    Status SendRequest(const std::string& file_name, OpCode opCode);
    Status SendAck(const std::string& host, uint16_t port);
    Status GetData(std::vector<BYTE>& buffer);
    Status PutData(const std::vector<BYTE>& data);
    Status Read(std::vector<BYTE>& buffer);
    static std::string GenerateTimeSuffix();

    static inline const std::string kDownloadedDefaultFname = "input";
    static inline const std::string kUploadedDefaultFname = "output";
    static const uint8_t kHeaderSize = 4; ///< TFTP header size in bytes (RFC 1350)
    static const uint16_t kDataMaxSize = 512; ///< Maximum data payload per packet (RFC 1350)
    static inline int kCallCounter = 0;
    static inline std::string kBaseFilename = GenerateTimeSuffix();

    UdpSocket* m_socket;
    // There is no ownership during testing
    bool m_ownsSocket;
    std::string m_remote_addr;
    uint16_t m_initial_port;
    // Note: tftp server changes port durind data exchange
    uint16_t m_remote_port; 
    uint16_t m_received_block_id;
};

#endif // TFTPCLIENT_H
