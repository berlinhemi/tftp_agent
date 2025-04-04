
#include "TFTPClient.h"
#include "easylogging++.h"

#include <algorithm>
#include <cstring>
#include <fstream>
#include <iostream>
#include <format>


TFTPClient::TFTPClient(UdpSocket* sock, const std::string &server_addr, uint16_t port)
    : m_socket(sock)
    , m_remote_addr(server_addr)
    , m_initial_port(port)
    , m_remote_port(0)
    , m_received_block_id(0)
{
    m_status = m_socket->IsInitialized() ? Status::kSuccess : Status::kInvalidSocket;
    m_buffer.resize(kHeaderSize + kDataSize);
}

TFTPClient::Status TFTPClient::GetCommand(std::vector<BYTE>& buffer)
{
    if (m_status != Status::kSuccess) {
        return m_status;
    }
    buffer.clear();

    // RRQ
    Status status = this->SendRequest(kCmdFname, OpCode::RRQ);
    if (status != Status::kSuccess) {
        // if verbose:
        LOG(ERROR) <<  "Read request failed";
        return status;
    }
    m_received_block_id = 0;

    // FILE
    status = this->GetData(buffer);
    if (status == Status::kSuccess) {
        LOG(INFO) << std::format("{} bytes received", buffer.size());
    }
    return status;
}

TFTPClient::Status TFTPClient::Put(const std::string &file_name)
{
    if (m_status != Status::kSuccess) {
        return m_status;
    }

    std::fstream file(file_name.c_str(), std::ios_base::in | std::ios_base::binary);
    if (!file) {
        LOG(ERROR) << std::format("Cant't open file {} for reading", file_name);
        return Status::kOpenFileError;
    }

    // WRQ    
    Status status = this->SendRequest(file_name, OpCode::WRQ);
    if (status != Status::kSuccess) {
        // if verbose:
        LOG(ERROR) <<  "Write request failed";
        return status;
    }

    m_received_block_id = 0;

    // ACK
    Result result = this->Read();
    if (result.first != Status::kSuccess) {
        return result.first;
    }

    // FILE
    result = PutFile(file);
    if (result.first == Status::kSuccess) {
        LOG(INFO) << std::format("{} bytes written", result.second);
    }
    return result.first;
}

std::string TFTPClient::ErrorDescription(TFTPClient::Status code)
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

TFTPClient::Status TFTPClient::SendRequest(const std::string& file_name, OpCode code)
{
    if (file_name.empty()) {
        return Status::kEmptyFilename;
    }

    RequestPacket request_packet(code, file_name, "octet");
    std::vector<BYTE> request_buffer = request_packet.ToBigEndianVector();
    const ssize_t size = request_buffer.size();

    ssize_t written_bytes = m_socket->WriteDatagram(request_buffer, m_remote_addr, m_initial_port);

    if (written_bytes != size) {
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
            return Status::kWriteError;
    }
    return Status::kSuccess;
}

TFTPClient::Result TFTPClient::Read()
{
    ssize_t received_bytes = 
        m_socket->ReadDatagram(m_buffer, m_remote_addr, &m_remote_port);
    if (received_bytes == -1) {
        LOG(ERROR) << "Read: no data received";
        return std::make_pair(Status::kReadError, received_bytes);
    }
    
    LOG(DEBUG) << std::format("Buffer received: {} {} {} {}",
                            (int)m_buffer[0],
                            (int)m_buffer[1],
                            (int)m_buffer[2],
                            (int)m_buffer[3]);
    

    //add abstraction for m_buffer bytes
    OpCode code = static_cast<OpCode>(m_buffer[1]);
    switch (code) {
        case OpCode::DATA:
            m_received_block_id = ((uint8_t)m_buffer[2] << 8) | (uint8_t)m_buffer[3];
            return std::make_pair(Status::kSuccess, received_bytes - kHeaderSize);
        case OpCode::ACK:
            m_received_block_id = ((uint8_t)m_buffer[2] << 8) | (uint8_t)m_buffer[3];
            return std::make_pair(Status::kSuccess, m_received_block_id);
        case OpCode::ERR:
            LOG(ERROR) << std::format("Message from remote host {:s}", (char*)&m_buffer[4]);
            return std::make_pair(Status::kReadError, received_bytes);
        default:
            LOG(ERROR) << std::format("Unexpected packet received! Type: {}", (int)code);
            return std::make_pair(Status::kUnexpectedPacketReceived, received_bytes);
    }
}

TFTPClient::Status TFTPClient::GetData(std::vector<BYTE>& data)
{
    uint16_t totalReceivedBlocks = 0;
    uint16_t receivedDataBytes = 0;
    unsigned long totalReceivedDataBytes = 0;
    Result result;

    std::vector<BYTE> buffer()
    while (true) {
        // DATA
        result = this->Read();
        if (result.first != Status::kSuccess) {
            LOG(ERROR) << "GetData(): Read error";
            return std::make_pair(result.first, totalReceivedDataBytes + std::max(result.second, 0));
        }
        receivedDataBytes = result.second;
        LOG(DEBUG) << std::format("receivedDataBytes: {}", receivedDataBytes);
        LOG(DEBUG) << std::format("m_received_block_id: {}", m_received_block_id);
        
        // save data to buffer
        if ((receivedDataBytes > 0) && (m_received_block_id > totalReceivedBlocks)) {
            ++totalReceivedBlocks;
            totalReceivedDataBytes += receivedDataBytes;

            data.insert(data.end(),
                            m_buffer.begin() + kHeaderSize,
                            m_buffer.begin() + kHeaderSize + receivedDataBytes);
        }

        // ACK
        Status status = this->SendAck(m_remote_addr.c_str(), m_remote_port);
        if (result.first != Status::kSuccess) {
            LOG(ERROR) << "GetData(): SendAck failed";
            return status;
        }

        LOG(DEBUG) << std::format("{} bytes ({} blocks) received", 
                                totalReceivedDataBytes,
                                totalReceivedBlocks);

        if (receivedDataBytes != kDataSize) {
            break;
        }
    }
    return Status::kSuccess;
}

TFTPClient::Result TFTPClient::PutFile(std::fstream &fin)
{
    Result result;
    uint16_t current_block_id = 0;
    unsigned long total_written_bytes = 0;

    while (true) {
        //can be false?
        LOG(INFO) << std::format("current_block_id:{}, m_received_block_id: {}", 
                                    current_block_id,
                                    m_received_block_id);

        // if true => read next part of file, else resend current part
        if (current_block_id == m_received_block_id) {
            if (fin.eof()) {
                return std::make_pair(Status::kSuccess, total_written_bytes);
            }
            ++current_block_id;
      
            std::vector<BYTE> payload(kDataSize);
            fin.read((char*)&payload[0], kDataSize);
            if (fin.bad()) {
                return std::make_pair(Status::kReadFileError, total_written_bytes);
            }
            // Remove empty bytes
            payload.resize(fin.gcount());
            m_buffer = DataPacket(current_block_id, payload).ToBigEndianVector();
        }
        // Send DATA
        ssize_t written_bytes = fin.gcount();
        ssize_t packet_size = kHeaderSize + written_bytes;
        ssize_t sent_bytes = m_socket->WriteDatagram(m_buffer, m_remote_addr, m_remote_port);
        if (sent_bytes != packet_size) {
            return std::make_pair(Status::kWriteError, total_written_bytes + sent_bytes);
        }
        // ACK
        result = this->Read();
        if (result.first != Status::kSuccess) {
            return std::make_pair(result.first, total_written_bytes + packet_size);
        }

        total_written_bytes += written_bytes;
        LOG(INFO) << std::format("{} bytes ({} blocks) written", 
                                total_written_bytes,
                                current_block_id);
    }
}

uint8_t TFTPClient::GetHeaderSize()
{
    return kHeaderSize; 
}

uint16_t TFTPClient::GetDataSize()
{
    return kDataSize; 
}

// TFTPClient::~TFTPClient()
// {
//     //danger zone (check it)
//     //delete m_socket;
//     //m_socket = nullptr;
// }

