#include "tftp_client.h"

#include <algorithm>
#include <cstring>
#include <fstream>
#include <iostream>

TFTPClient::TFTPClient(const std::string &server_addr, uint16_t port)
    : remote_addr_(server_addr)
    , port_(port)
    , remote_port_(0)
    , received_block_(0)
{
    socket_ = new UdpSocket();
    status_ = socket_->Init() ? Status::kSuccess : Status::kInvalidSocket;
}

TFTPClient::TFTPClient(UdpSocket* sock, const std::string &server_addr, uint16_t port)
    : socket_(sock)
    //Not good (for tests only)
    , status_(Status::kSuccess)
    , remote_addr_(server_addr)
    , port_(port)
    , remote_port_(0)
    , received_block_(0)
{}

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
    received_block_ = 0;

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

    received_block_ = 0;

    // ACK
    result = this->Read();
    if (result.first != Status::kSuccess) {
        return result.first;
    }

    // FILE
    result = PutFile(file);
    if (result.first == Status::kSuccess) {
        //std::cout << "kSuccess: " << result.second << " bytes written!\n";
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

    const std::string mode("octet");

    buffer_[0] = 0;
    buffer_[1] = static_cast<char>(code);

    // filename
    char *end = strcpy((char*)&buffer_[2], file_name.c_str());
    end += file_name.size();
    end++;
    // mode
    end = std::strcpy(end, mode.c_str());
    end += mode.size();
    end++;
    
    ssize_t size = std::distance((char*)&buffer_[0], end); 
    std::vector<BYTE> data (buffer_.begin(), buffer_.begin() + size);
    ssize_t written_bytes = socket_->WriteDatagram(data, remote_addr_, port_);
    std::cout << written_bytes << std::endl;
    std::cout << size << std::endl;
    //std::cout << "IAM HERE! writtenBytes:" << written_bytes << std::endl;
    if (written_bytes != size) {
        return std::make_pair(Status::kWriteError, written_bytes);
    }

    return std::make_pair(Status::kSuccess, written_bytes);
}

TFTPClient::Result TFTPClient::SendAck(const std::string& host, uint16_t port)
{
    const std::size_t data_size = 4;

    buffer_[0] = 0;
    buffer_[1] = static_cast<BYTE>(OpCode::ACK);
    //next 2 bytes is containing received block id, do not overwrite them
    
    std::vector<BYTE> data (buffer_.begin(), buffer_.begin() + data_size);
    ssize_t bytes_written = socket_->WriteDatagram(data, host, port);

    bool is_success = bytes_written == data_size;
    return std::make_pair(is_success ? Status::kSuccess : Status::kWriteError,
                          is_success ? data_size : bytes_written);
}

TFTPClient::Result TFTPClient::Read()
{
    std::vector<BYTE> tmp_buffer (buffer_.size());
    ssize_t received_bytes = 
        socket_->ReadDatagram(tmp_buffer, tmp_buffer.size(), remote_addr_, &remote_port_);
    if (received_bytes == -1) {
        std::puts("\nError! No data received.");
        return std::make_pair(Status::kReadError, received_bytes);
    }
    //append received datagram to buffer_
    std::copy_n(tmp_buffer.begin(), tmp_buffer.size(), buffer_.begin());

    OpCode code = static_cast<OpCode>(buffer_[1]);
    switch (code) {
    case OpCode::DATA:
        received_block_ = ((uint8_t)buffer_[2] << 8) | (uint8_t)buffer_[3];
        return std::make_pair(Status::kSuccess, received_bytes - kHeaderSize);
    case OpCode::ACK:
        received_block_ = ((uint8_t)buffer_[2] << 8) | (uint8_t)buffer_[3];
        return std::make_pair(Status::kSuccess, received_block_);
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
            return std::make_pair(result.first, totalReceivedDataBytes + std::max(result.second, 0));
        }
        receivedDataBytes = result.second;

        // write to file
        if ((receivedDataBytes > 0) && (received_block_ > totalReceivedBlocks)) {
            ++totalReceivedBlocks;
            totalReceivedDataBytes += receivedDataBytes;

            file.write((char*)&buffer_[kHeaderSize], receivedDataBytes);
            if (file.bad()) {
                return std::make_pair(Status::kWriteFileError, totalReceivedDataBytes);
            }
        }

        // ACK
        result = this->SendAck(remote_addr_.c_str(), remote_port_);
        if (result.first != Status::kSuccess) {
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
    uint16_t current_block = 0;
    unsigned long total_written_bytes = 0;

    while (true) {
        if (current_block == received_block_) {
            if (file.eof()) {
                return std::make_pair(Status::kSuccess, total_written_bytes);
            }
            ++current_block;

            buffer_[0] = 0;
            buffer_[1] = static_cast<char>(OpCode::DATA);
            buffer_[2] = static_cast<uint8_t>(current_block >> 8);
            buffer_[3] = static_cast<uint8_t>(current_block & 0x00FF);

            // read from file
            file.read((char*)&buffer_[kHeaderSize], kDataSize);
            if (file.bad()) {
                return std::make_pair(Status::kReadFileError, total_written_bytes);
            }
        }

        // DATA
        ssize_t packet_size = kHeaderSize + file.gcount();
        std::vector<BYTE> data (buffer_.begin(), buffer_.begin() + packet_size);
        ssize_t written_bytes = socket_->WriteDatagram(data, remote_addr_, remote_port_);
        if (written_bytes != packet_size) {
            return std::make_pair(Status::kWriteError, total_written_bytes + written_bytes);
        }

        // ACK
        result = this->Read();
        if (result.first != Status::kSuccess) {
            return std::make_pair(result.first, total_written_bytes + packet_size);
        }

        total_written_bytes += file.gcount();

        printf("\r%lu bytes (%i blocks) written", total_written_bytes, current_block);
    }
}

TFTPClient::~TFTPClient()
{
    //danger zone (check it)
    //delete socket_;
    //socket_ = nullptr;
}

