
#include "TFTPClient.h"

#include <algorithm>
#include <cstring>
#include <fstream>
#include <iostream>

// TFTPClient::TFTPClient(const std::string &server_addr, uint16_t port)
//     : m_remote_addr(server_addr)
//     , m_initial_port(port)
//     , m_remote_port(0)
//     , m_received_block_id(0)
// {
//     m_socket = new UdpSocket();
//     m_status = m_socket->Init() ? Status::kSuccess : Status::kInvalidSocket;
// }

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

TFTPClient::Status TFTPClient::Get(const std::string &file_name)
{
    if (m_status != Status::kSuccess) {
        return m_status;
    }

    std::string out_fname = file_name + ".received";
    std::fstream file(out_fname.c_str(), std::ios_base::out | std::ios_base::binary);
    if (!file) {
        std::puts("\nError! Cant't opening file for writing!");
        return Status::kOpenFileError;
    }

    // RRQ
    Result result = this->SendRequest(file_name, OpCode::RRQ);
    if (result.first != Status::kSuccess) {
        return result.first;
    }
    m_received_block_id = 0;

    // FILE
    result = this->GetFile(file);
    if (result.first == Status::kSuccess) {
        std::cout << "kSuccess: " << result.second << " bytes received!\n";
    }
    return result.first;
}

TFTPClient::Status TFTPClient::Put(const std::string &file_name)
{
    if (m_status != Status::kSuccess) {
        return m_status;
    }

    std::fstream file(file_name.c_str(), std::ios_base::in | std::ios_base::binary);
    if (!file) {
        std::puts("\nError! Can't open file for reading!");
        return Status::kOpenFileError;
    }

    // WRQ
    Result result = this->SendRequest(file_name, OpCode::WRQ);
    if (result.first != Status::kSuccess) {
        return result.first;
    }

    m_received_block_id = 0;

    // ACK
    result = this->Read();
    if (result.first != Status::kSuccess) {
        return result.first;
    }

    // FILE
    result = PutFile(file);
    if (result.first == Status::kSuccess) {
        std::cout << "kSuccess: " << result.second << " bytes written!\n";
    }
    return result.first;
}

std::string TFTPClient::ErrorDescription(TFTPClient::Status code)
{
    switch (code) {
    case Status::kSuccess:
        return "\nSuccess.";
    case Status::kInvalidSocket:
        return "\nError! Invalid Socket.";
    case Status::kWriteError:
        return "\nError! Write Socket.";
    case Status::kReadError:
        return "\nError! Read Socket.";
    case Status::kUnexpectedPacketReceived:
        return "\nError! Unexpected Packet Received.";
    case Status::kEmptyFilename:
        return "\nError! Empty Filename.";
    case Status::kOpenFileError:
        return "\nError! Can't Open File.";
    case Status::kWriteFileError:
        return "\nError! Write File.";
    case Status::kReadFileError:
        return "\nError! Read File.";
    default:
        return "\nError!";
    }
}

TFTPClient::Result TFTPClient::SendRequest(const std::string& file_name, OpCode code)
{
    if (file_name.empty()) {
        return std::make_pair(Status::kEmptyFilename, 0);
    }

    RequestPacket request_packet(code, file_name, "octet");
    std::vector<BYTE> request_buffer = request_packet.ToBigEndianVector();
    const ssize_t size = request_buffer.size();

    ssize_t written_bytes = m_socket->WriteDatagram(request_buffer, m_remote_addr, m_initial_port);

    if (written_bytes != size) {
        return std::make_pair(Status::kWriteError, written_bytes);
    }
    return std::make_pair(Status::kSuccess, written_bytes);
}

TFTPClient::Result TFTPClient::SendAck(const std::string& host, uint16_t port)
{
    AckPacket ack_packet(m_received_block_id);
    std::vector<BYTE> ack_buffer = ack_packet.ToBigEndianVector();
    const ssize_t size = ack_buffer.size();

    ssize_t bytes_written = m_socket->WriteDatagram(ack_buffer, host, port);

    if (bytes_written != size) {
            return std::make_pair(Status::kWriteError, bytes_written);
    }
    return std::make_pair(Status::kSuccess, bytes_written);
}

TFTPClient::Result TFTPClient::Read()
{
    
    ssize_t received_bytes = 
        m_socket->ReadDatagram(m_buffer, m_remote_addr, &m_remote_port);
    if (received_bytes == -1) {
        std::puts("\nError! No data received.");
        return std::make_pair(Status::kReadError, received_bytes);
    }
    // std::cout <<"tmp_buffer received:" << (int)tmp_buffer[0] 
    //     << "," << (int)tmp_buffer[1] 
    //     << "," << (int)tmp_buffer[2]
    //     << "," << (int)tmp_buffer[3]
    //     << std::endl
    //     << "received_bytes:" << received_bytes 
    //     << std::endl;

    //copy received datagram to m_buffer
    //std::copy_n(tmp_buffer.begin(), received_bytes, m_buffer.begin());

    std::cout <<"buffer received:" << (int)m_buffer[0] 
        << "," << (int)m_buffer[1] 
        << "," << (int)m_buffer[2]
        << "," << (int)m_buffer[3]
        << std::endl;

    //add abstaction ?
    OpCode code = static_cast<OpCode>(m_buffer[1]);
    switch (code) {
    case OpCode::DATA:
        m_received_block_id = ((uint8_t)m_buffer[2] << 8) | (uint8_t)m_buffer[3];
        return std::make_pair(Status::kSuccess, received_bytes - kHeaderSize);
    case OpCode::ACK:
        m_received_block_id = ((uint8_t)m_buffer[2] << 8) | (uint8_t)m_buffer[3];
        return std::make_pair(Status::kSuccess, m_received_block_id);
    case OpCode::ERR:
        printf("\nError! Message from remote host: %s", &m_buffer[4]);
        return std::make_pair(Status::kReadError, received_bytes);
    default:
        printf("\nError! Unexpected packet received! Type: %i", code);
        return std::make_pair(Status::kUnexpectedPacketReceived, received_bytes);
    }
}

TFTPClient::Result TFTPClient::GetFile(std::fstream &file)
{
    uint16_t totalReceivedBlocks = 0;
    uint16_t receivedDataBytes = 0;
    unsigned long totalReceivedDataBytes = 0;
    Result result;

    while (true) {
        // DATA
        result = this->Read();
        if (result.first != Status::kSuccess) {
            std::cout << "Read error"  << std::endl;
            return std::make_pair(result.first, totalReceivedDataBytes + std::max(result.second, 0));
        }
        receivedDataBytes = result.second;
        std::cout << "receivedDataBytes:" << receivedDataBytes << std::endl;
         std::cout << "m_received_block_id:" << m_received_block_id << std::endl;
        
        // write to file
        if ((receivedDataBytes > 0) && (m_received_block_id > totalReceivedBlocks)) {
            ++totalReceivedBlocks;
            totalReceivedDataBytes += receivedDataBytes;

            file.write((char*)&m_buffer[kHeaderSize], receivedDataBytes);
            if (file.bad()) {
                //std::cout << "file.bad." << std::endl;
                return std::make_pair(Status::kWriteFileError, totalReceivedDataBytes);
            }
        }

        // ACK
        result = this->SendAck(m_remote_addr.c_str(), m_remote_port);
        if (result.first != Status::kSuccess) {
            std::cout << "SendAck failed." << std::endl;
            return std::make_pair(result.first, totalReceivedDataBytes);
        }

        printf("\r%lu bytes (%i blocks) received", totalReceivedDataBytes, totalReceivedBlocks);

        if (receivedDataBytes != kDataSize) {
            break;
        }
    }
    return std::make_pair(Status::kSuccess, totalReceivedDataBytes);
}

TFTPClient::Result TFTPClient::PutFile(std::fstream &file)
{
    Result result;
    uint16_t current_block_id = 0;
    unsigned long total_written_bytes = 0;

    while (true) {
        //can be false?
        if (current_block_id == m_received_block_id) {
            if (file.eof()) {
                return std::make_pair(Status::kSuccess, total_written_bytes);
            }
            ++current_block_id;
      
            std::vector<BYTE> payload(kDataSize);
            file.read((char*)&payload[0], kDataSize);
            if (file.bad()) {
                return std::make_pair(Status::kReadFileError, total_written_bytes);
            }

            m_buffer = DataPacket(current_block_id, payload).ToBigEndianVector();
        }
        ssize_t written_bytes = file.gcount();
        // Send DATA
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
        printf("\r%lu bytes (%i blocks) written", total_written_bytes, current_block_id);
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

