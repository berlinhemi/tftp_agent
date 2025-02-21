#include "TFTPClient.h"

#include <algorithm>
#include <cstring>
#include <fstream>
#include <iostream>

// TFTPClient::TFTPClient(const std::string &server_addr, uint16_t port)
//     : remote_addr_(server_addr)
//     , initial_port_(port)
//     , remote_port_(0)
//     , received_block_id_(0)
// {
//     socket_ = new UdpSocket();
//     status_ = socket_->Init() ? Status::kSuccess : Status::kInvalidSocket;
// }

TFTPClient::TFTPClient(UdpSocket* sock, const std::string &server_addr, uint16_t port)
    : socket_(sock)
    , status_(Status::kSuccess)
    , remote_addr_(server_addr)
    , initial_port_(port)
    , remote_port_(0)
    , received_block_id_(0)
{
    status_ = socket_->IsInitialized() ? Status::kSuccess : Status::kInvalidSocket;
    buffer_.resize(kHeaderSize + kDataSize);
}

TFTPClient::Status TFTPClient::Get(const std::string &file_name)
{
    if (status_ != Status::kSuccess) {
        return status_;
    }

    std::fstream file(file_name.c_str(), std::ios_base::out | std::ios_base::binary);
    if (!file) {
        std::puts("\nError! Cant't opening file for writing!");
        return Status::kOpenFileError;
    }

    // RRQ
    Result result = this->SendRequest(file_name, OpCode::RRQ);
    if (result.first != Status::kSuccess) {
        return result.first;
    }
    received_block_id_ = 0;

    // FILE
    result = this->GetFile(file);
    if (result.first == Status::kSuccess) {
        std::cout << "kSuccess: " << result.second << " bytes received!\n";
    }
    return result.first;
}

TFTPClient::Status TFTPClient::Put(const std::string &file_name)
{
    if (status_ != Status::kSuccess) {
        return status_;
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

    received_block_id_ = 0;

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

    ssize_t written_bytes = socket_->WriteDatagram(request_buffer, remote_addr_, initial_port_);

    if (written_bytes != size) {
        return std::make_pair(Status::kWriteError, written_bytes);
    }
    return std::make_pair(Status::kSuccess, written_bytes);
}

TFTPClient::Result TFTPClient::SendAck(const std::string& host, uint16_t port)
{
    AckPacket ack_packet(received_block_id_);
    std::vector<BYTE> ack_buffer = ack_packet.ToBigEndianVector();
    const ssize_t size = ack_buffer.size();

    ssize_t bytes_written = socket_->WriteDatagram(ack_buffer, host, port);

    if (bytes_written != size) {
            return std::make_pair(Status::kWriteError, bytes_written);
    }
    return std::make_pair(Status::kSuccess, bytes_written);
}

TFTPClient::Result TFTPClient::Read()
{
    //std::vector<BYTE> tmp_buffer;
    //tmp_buffer.reserve(buffer_.size());
    ssize_t received_bytes = 
        socket_->ReadDatagram(/*tmp_buffer*/buffer_, remote_addr_, &remote_port_);
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

    //copy received datagram to buffer_
    //std::copy_n(tmp_buffer.begin(), received_bytes, buffer_.begin());

    std::cout <<"buffer received:" << (int)buffer_[0] 
        << "," << (int)buffer_[1] 
        << "," << (int)buffer_[2]
        << "," << (int)buffer_[3]
        << std::endl;

    //add abstaction ?
    OpCode code = static_cast<OpCode>(buffer_[1]);
    switch (code) {
    case OpCode::DATA:
        received_block_id_ = ((uint8_t)buffer_[2] << 8) | (uint8_t)buffer_[3];
        return std::make_pair(Status::kSuccess, received_bytes - kHeaderSize);
    case OpCode::ACK:
        received_block_id_ = ((uint8_t)buffer_[2] << 8) | (uint8_t)buffer_[3];
        return std::make_pair(Status::kSuccess, received_block_id_);
    case OpCode::ERR:
        printf("\nError! Message from remote host: %s", &buffer_[4]);
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
         std::cout << "received_block_id_:" << received_block_id_ << std::endl;
        
        // write to file
        if ((receivedDataBytes > 0) && (received_block_id_ > totalReceivedBlocks)) {
            ++totalReceivedBlocks;
            totalReceivedDataBytes += receivedDataBytes;

            file.write((char*)&buffer_[kHeaderSize], receivedDataBytes);
            if (file.bad()) {
                //std::cout << "file.bad." << std::endl;
                return std::make_pair(Status::kWriteFileError, totalReceivedDataBytes);
            }
        }

        // ACK
        result = this->SendAck(remote_addr_.c_str(), remote_port_);
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
        if (current_block_id == received_block_id_) {
            if (file.eof()) {
                return std::make_pair(Status::kSuccess, total_written_bytes);
            }
            ++current_block_id;
      
            // buffer_[0] = 0;
            // buffer_[1] = static_cast<char>(OpCode::DATA);
            // buffer_[2] = static_cast<uint8_t>(current_block_id >> 8);
            // buffer_[3] = static_cast<uint8_t>(current_block_id & 0x00FF);
            std::vector<BYTE> payload(kDataSize);
            file.read((char*)&payload[0], kDataSize);
            // read from file
            //file.read((char*)&buffer_[kHeaderSize], kDataSize);
            if (file.bad()) {
                return std::make_pair(Status::kReadFileError, total_written_bytes);
            }
            DataPacket dataPacket(current_block_id, payload);
            buffer_ = dataPacket.ToBigEndianVector();
        }

        // DATA
        ssize_t packet_size = kHeaderSize + file.gcount();
        //std::vector<BYTE> data (buffer_.begin(), buffer_.begin() + packet_size);
        //ssize_t written_bytes = socket_->WriteDatagram(data, remote_addr_, remote_port_);
        ssize_t written_bytes = socket_->WriteDatagram(buffer_, remote_addr_, remote_port_);
        if (written_bytes != packet_size) {
            return std::make_pair(Status::kWriteError, total_written_bytes + written_bytes);
        }

        // ACK
        result = this->Read();
        if (result.first != Status::kSuccess) {
            return std::make_pair(result.first, total_written_bytes + packet_size);
        }

        total_written_bytes += file.gcount();

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


TFTPClient::~TFTPClient()
{
    //danger zone (check it)
    //delete socket_;
    //socket_ = nullptr;
}

