
#include "TFTPClient.h"
#include "easylogging++.h"

#include <algorithm>
#include <cstring>
#include <fstream>
#include <iostream>
#include <format>
#include <cassert>


TFTPClient::TFTPClient(UdpSocket* sock, const std::string &server_addr, uint16_t port)
    : m_socket(sock)
    , m_remote_addr(server_addr)
    , m_initial_port(port)
    , m_remote_port(0)
    , m_received_block_id(0)
{
    if (!m_socket->IsInitialized())
    {
        throw std::runtime_error("Invalid socket");
    }
}

TFTPClient::Status TFTPClient::GetCommand(std::vector<BYTE>& buffer)
{
    // RRQ
    Status status = SendRequest(kCmdFname, OpCode::RRQ);
    if (status != Status::kSuccess) {
        LOG(ERROR) <<  "GetCommand(): failed to send read request";
        return status;
    }

    m_received_block_id = 0;

    // DATA
    status = GetData(buffer);
    if (status == Status::kSuccess) {
        //if verbose
        LOG(INFO) << std::format("GetCommand(): total {} bytes received", buffer.size());
    }
    return status;
}


TFTPClient::Status TFTPClient::PutResults(const std::vector<BYTE>& data)
{
    // WRQ    
    Status status = SendRequest(kResultFname, OpCode::WRQ);
    if (status != Status::kSuccess) {
        LOG(ERROR) <<  "PutResults(): failed to send write request";
        return status;
    }

    m_received_block_id = 0;

    // ACK
    std::vector<BYTE> ack_buffer;
    status = Read(ack_buffer);
    if (status != Status::kSuccess) {
        return status;
    }

    // DATA
    status = PutData(data);
    if (status == Status::kSuccess) {
        //if verbose
        LOG(INFO) << std::format("PutResults(): {} bytes written", data.size());

    }
    return status;
}


std::string TFTPClient::ErrorDescription(TFTPClient::Status code) const
{
    switch (code) {
    case Status::kSuccess:
        return "Success";
    case Status::kInvalidSocket:
        return "Socket is not initialized";
    case Status::kWriteError:
        return "Write Socket error";
    case Status::kReadError:
        return "Read Socket error";
    case Status::kUnexpectedPacketReceived:
        return "Unexpected Packet Received";
    case Status::kEmptyFilename:
        return "Empty Filename";
    case Status::kOpenFileError:
        return "Can't Open File";
    case Status::kWriteFileError:
        return "Write File error";
    case Status::kReadFileError:
        return "Read File error";
    case Status::kSendRequestError:
        return "Send read/write request error";
    default:
        return "Unidentified error";
    }
}

TFTPClient::Status TFTPClient::SendRequest(const std::string& file_name, OpCode opCode)
{
    if (file_name.empty()) {
        return Status::kEmptyFilename;
    }

    RequestPacket request_packet(opCode, file_name, "octet");
    std::vector<BYTE> request_buffer = request_packet.ToBigEndianVector();

    ssize_t written_bytes = m_socket->WriteDatagram(request_buffer, m_remote_addr, m_initial_port);

    if (written_bytes != request_buffer.size()) {
        LOG(ERROR) << "SendRequest(): incorrect amount of data transmitted";
        return Status::kSendRequestError;
    }

    return Status::kSuccess;
}

TFTPClient::Status TFTPClient::SendAck(const std::string& host, uint16_t port)
{
    AckPacket ack_packet(m_received_block_id);
    std::vector<BYTE> ack_buffer = ack_packet.ToBigEndianVector();
    const ssize_t size = ack_buffer.size();

    ssize_t bytes_written = m_socket->WriteDatagram(ack_buffer, host, port);

    if (bytes_written != size) {
        LOG(ERROR) << "SendAck(): incorrect amount of data transmitted";
        return Status::kWriteError;
    }
    return Status::kSuccess;
}

TFTPClient::Status TFTPClient::Read(std::vector<BYTE>& buffer)
{
    const size_t max_packet_len = kHeaderSize + kDataMaxSize;
    ssize_t received_bytes = 
        m_socket->ReadDatagram(buffer, max_packet_len, m_remote_addr, &m_remote_port);

    if (received_bytes == -1) 
    {
        LOG(ERROR) << "Read(): no bytes received";
        return Status::kReadError;
    }
    
    if(buffer.size() < kHeaderSize)
    {
        LOG(ERROR) << "Read(): not enough bytes received";
        return Status::kReadError;
    }

    // if verbose
    LOG(DEBUG) << std::format("Header bytes received: {} {} {} {}",
                            (int)buffer[0],
                            (int)buffer[1],
                            (int)buffer[2],
                            (int)buffer[3]);
    

    // add abstraction for buffer bytes ?
    OpCode code = static_cast<OpCode>(buffer[1]);
    switch (code) {
        case OpCode::DATA:
            m_received_block_id = ((uint8_t)buffer[2] << 8) | (uint8_t)buffer[3];
            return Status::kSuccess;
        case OpCode::ACK:
            m_received_block_id = ((uint8_t)buffer[2] << 8) | (uint8_t)buffer[3];
            return Status::kSuccess;
        case OpCode::ERR:
            LOG(ERROR) << std::format("Read(): error message from remote host: {:s}", (char*)&buffer[4]);
            return Status::kReadError;
        default:
            LOG(ERROR) << std::format("Read(): unexpected packet received. Type: {}", (int)code);
            return Status::kUnexpectedPacketReceived;
    }
}

TFTPClient::Status TFTPClient::GetData(std::vector<BYTE>& data)
{
    uint16_t totalReceivedBlocks = 0;
    uint16_t receivedDataBytes = 0;
    unsigned long totalReceivedDataBytes = 0;
    Status status;

    data.clear();
    std::vector<BYTE> buffer;
    do {
        // DATA
        status = Read(buffer);
        if (status != Status::kSuccess) {
            LOG(ERROR) << "GetData(): Read error";
            return status;
        }
        receivedDataBytes = buffer.size();
        //if verbose
        LOG(DEBUG) << std::format("receivedDataBytes: {}", receivedDataBytes);
        LOG(DEBUG) << std::format("m_received_block_id: {}", m_received_block_id);
        
        // Save data to buffer
        if ((receivedDataBytes > 0) && (m_received_block_id > totalReceivedBlocks)) {

            ++totalReceivedBlocks;
            totalReceivedDataBytes += receivedDataBytes;

            data.insert(data.end(),
                        buffer.begin() + kHeaderSize,
                        buffer.begin() + kHeaderSize + receivedDataBytes);
        }

        // ACK
        status = SendAck(m_remote_addr.c_str(), m_remote_port);
        if (status != Status::kSuccess) {
            LOG(ERROR) << "GetData(): SendAck failed";
            return status;
        }

        LOG(DEBUG) << std::format("{} bytes ({} blocks) received", 
                                totalReceivedDataBytes,
                                totalReceivedBlocks);


    } while(receivedDataBytes == kDataMaxSize);
    
    return Status::kSuccess;
}

TFTPClient::Status TFTPClient::PutData(const std::vector<BYTE>& data)
{
    //Result result;
    Status status;
    uint16_t current_block_id = 0;
    size_t datachunk_size;
    size_t written_bytes = 0;
    std::vector<BYTE> buffer;

    while (true) {
        LOG(DEBUG) << std::format("PutData(): current_block_id:{}, m_received_block_id: {}", 
                                current_block_id,
                                m_received_block_id);

        // if true => read next chunk of data to buffer, otherwise resend buffer
        if (current_block_id == m_received_block_id) {
            size_t remaining_bytes = data.size() - written_bytes;
            // All bytes are sent
            if(remaining_bytes < 1){
                return Status::kSuccess;
            }

            current_block_id++;
            datachunk_size = remaining_bytes < kDataMaxSize ? remaining_bytes : kDataMaxSize;
            // Extract next chunk for sending
            std::vector<BYTE> payload(data.begin() + written_bytes,
                                      data.begin() + written_bytes + datachunk_size);                    
            
            buffer = DataPacket(current_block_id, payload).ToBigEndianVector();
            written_bytes += datachunk_size;
        }

        // Send DATA
        ssize_t packet_size = kHeaderSize + datachunk_size;
        ssize_t sent_bytes = m_socket->WriteDatagram(buffer, m_remote_addr, m_remote_port);
        if (sent_bytes != packet_size) {
            // if verbose
            LOG(ERROR) << "PutData(): Send DATA failed";
            return Status::kWriteError;
        }

        // ACK
        std::vector<BYTE> ack_buffer;
        status = Read(ack_buffer);
        if (status != Status::kSuccess) {
            // if verbose
            LOG(ERROR) << "PutData(): failed to read ack";
            return status;
        }

        // if verbose
        LOG(INFO) << std::format("PutData(): {} bytes ({} blocks) written", 
                                written_bytes,
                                current_block_id);
    }
}



uint8_t TFTPClient::GetHeaderSize() 
{
    return kHeaderSize; 
}

uint16_t TFTPClient::GetDataSize() 
{
    return kDataMaxSize; 
}

std::string  TFTPClient::GetCommandFName() 
{
    return kCmdFname;
}
std::string  TFTPClient::GetResultFName() 
{
    return kResultFname;
}

// TFTPClient::~TFTPClient()
// {
//     //danger zone (check it)
//     //delete m_socket;
//     //m_socket = nullptr;
// }

