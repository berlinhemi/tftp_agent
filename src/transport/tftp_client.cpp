#include "tftp_client.h"

#include <cstring>
#include <fstream>

TFTPClient::TFTPClient(const std::string &server_addr, uint16_t port)
    : remote_addr_(server_addr)
    , port_(port)
    , remote_port_(0)
    , received_block_(0)
{
    status_ = socket_.Init() ? Status::kSuccess : Status::kInvalidSocket;
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

    received_block_ = 0;

    // FILE
    result = this->GetFile(file);
    if (result.first == Status::kSuccess) {
        //std::cout << "kSuccess: " << result.second << " bytes received!\n";
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

TFTPClient::Result TFTPClient::SendRequest(const std::string &file_name, OpCode code)
{
    if (file_name.empty()) {
        return std::make_pair(Status::kEmptyFilename, 0);
    }

    std::string mode("octet");

    buffer_[0] = 0;
    buffer_[1] = static_cast<char>(code);

    // filename
    char *end = std::strncpy(&buffer_[2], file_name.c_str(), file_name.size()) + file_name.size();
    *end++ = '\0';

    // mode
    end = std::strncpy(end, mode.c_str(), mode.size()) + mode.size();
    *end++ = '\0';

    const auto packetSize = std::distance(&buffer_[0], end);
    const auto writtenBytes =
            socket_.WriteDatagram(&buffer_[0], packetSize, remote_addr_.c_str(), port_);
    if (writtenBytes != packetSize) {
        return std::make_pair(Status::kWriteError, writtenBytes);
    }

    return std::make_pair(Status::kSuccess, writtenBytes);
}

TFTPClient::Result TFTPClient::SendAck(const char *host, uint16_t port)
{
    const std::size_t packetSize = 4;

    buffer_[0] = 0;
    buffer_[1] = static_cast<char>(OpCode::ACK);

    const auto bytesWritten =
            socket_.WriteDatagram(&buffer_[0], packetSize, host, port);
    const bool isSuccess = bytesWritten == packetSize;

    return std::make_pair(isSuccess ? Status::kSuccess : Status::kWriteError,
                          isSuccess ? packetSize : bytesWritten);
}

TFTPClient::Result TFTPClient::Read()
{
    const auto receivedBytes =
            socket_.ReadDatagram(&buffer_[0], buffer_.size(), &remote_addr_[0], &remote_port_);
    if (receivedBytes == -1) {
        std::puts("\nError! No data received.");
        return std::make_pair(Status::kReadError, receivedBytes);
    }

    const auto code = static_cast<OpCode>(buffer_[1]);
    switch (code) {
    case OpCode::DATA:
        received_block_ = ((uint8_t)buffer_[2] << 8) | (uint8_t)buffer_[3];
        return std::make_pair(Status::kSuccess, receivedBytes - kHeaderSize);
    case OpCode::ACK:
        received_block_ = ((uint8_t)buffer_[2] << 8) | (uint8_t)buffer_[3];
        return std::make_pair(Status::kSuccess, received_block_);
    case OpCode::ERR:
        printf("\nError! Message from remote host: %s", &buffer_[4]);
        return std::make_pair(Status::kReadError, receivedBytes);
    default:
        printf("\nError! Unexpected packet received! Type: %i", code);
        return std::make_pair(Status::kUnexpectedPacketReceived, receivedBytes);
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

            file.write(&buffer_[kHeaderSize], receivedDataBytes);
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
    uint16_t currentBlock = 0;
    unsigned long totalWrittenBytes = 0;

    while (true) {
        if (currentBlock == received_block_) {
            if (file.eof()) {
                return std::make_pair(Status::kSuccess, totalWrittenBytes);
            }
            ++currentBlock;

            buffer_[0] = 0;
            buffer_[1] = static_cast<char>(OpCode::DATA);
            buffer_[2] = static_cast<uint8_t>(currentBlock >> 8);
            buffer_[3] = static_cast<uint8_t>(currentBlock & 0x00FF);

            // read from file
            file.read(&buffer_[kHeaderSize], kDataSize);
            if (file.bad()) {
                return std::make_pair(Status::kReadFileError, totalWrittenBytes);
            }
        }

        // DATA
        const auto packetSize = kHeaderSize + file.gcount();
        const auto writtenBytes =
                socket_.WriteDatagram(&buffer_[0], packetSize, remote_addr_.c_str(), remote_port_);
        if (writtenBytes != packetSize) {
            return std::make_pair(Status::kWriteError, totalWrittenBytes + writtenBytes);
        }

        // ACK
        result = this->Read();
        if (result.first != Status::kSuccess) {
            return std::make_pair(result.first, totalWrittenBytes + packetSize);
        }

        totalWrittenBytes += file.gcount();

        printf("\r%lu bytes (%i blocks) written", totalWrittenBytes, currentBlock);
    }
}
