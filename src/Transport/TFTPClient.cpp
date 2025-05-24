
#include "TFTPClient.h"
#include "easylogging++.h"

#include <algorithm>
#include <cstring>
#include <fstream>
#include <iostream>
#include <format>
<<<<<<< HEAD

// TFTPClient::TFTPClient(const std::string &server_addr, uint16_t port)
//     : m_remote_addr(server_addr)
//     , m_initial_port(port)
//     , m_remote_port(0)
//     , m_received_block_id(0)
// {
//     m_socket = new UdpSocket();
//     m_status = m_socket->Init() ? Status::kSuccess : Status::kInvalidSocket;
// }
=======
#include <cassert>

>>>>>>> origin/dev

TFTPClient::TFTPClient(UdpSocket* sock, const std::string &server_addr, uint16_t port)
    : m_socket(sock)
    , m_remote_addr(server_addr)
    , m_initial_port(port)
    , m_remote_port(0)
    , m_received_block_id(0)
{
<<<<<<< HEAD
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
        LOG(ERROR) << std::format("Cant't open file {} for writing", out_fname);
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
        LOG(INFO) << std::format("{} bytes received", result.second);
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
        LOG(ERROR) << std::format("Cant't open file {} for reading", file_name);
        return Status::kOpenFileError;
    }

    // WRQ
    Result result = this->SendRequest(file_name, OpCode::WRQ);
    if (result.first != Status::kSuccess) {
        return result.first;
=======
    if (!m_socket->IsInitialized())
    {
        throw std::runtime_error("Invalid socket");
    }
}

TFTPClient::Status TFTPClient::Get(std::vector<BYTE>& buffer, const std::string& fname)
{
    // RRQ
    Status status = SendRequest(fname, OpCode::RRQ);
    if (status != Status::kSuccess) {
        LOG(ERROR) <<  "Get(): failed to send read request";
        return status;
    }

    m_received_block_id = 0;

    // DATA
    status = GetData(buffer);
    if (status == Status::kSuccess) {
        //if verbose
        LOG(INFO) << std::format("Get(): total {} bytes received", buffer.size());
    }
    return status;
}


TFTPClient::Status TFTPClient::Put(const std::vector<BYTE>& data, const std::string& fname)
{
    // WRQ    
    Status status = SendRequest(fname, OpCode::WRQ);
    if (status != Status::kSuccess) {
        LOG(ERROR) <<  "Put(): failed to send write request";
        return status;
>>>>>>> origin/dev
    }

    m_received_block_id = 0;

    // ACK
<<<<<<< HEAD
    result = this->Read();
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
=======
    std::vector<BYTE> ack_buffer;
    status = Read(ack_buffer);
    if (status != Status::kSuccess) {
        return status;
    }

    // DATA
    status = PutData(data);
    if (status == Status::kSuccess) {
        //if verbose
        LOG(INFO) << std::format("Put(): {} bytes written", data.size());
    }
    return status;
}


std::string TFTPClient::ErrorDescription(TFTPClient::Status code) const
>>>>>>> origin/dev
{
    switch (code) {
    case Status::kSuccess:
        return "Success";
    case Status::kInvalidSocket:
<<<<<<< HEAD
        return "Invalid Socket error";
=======
        return "Socket is not initialized";
>>>>>>> origin/dev
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
<<<<<<< HEAD
=======
    case Status::kSendRequestError:
        return "Send read/write request error";
>>>>>>> origin/dev
    default:
        return "Unidentified error";
    }
}

<<<<<<< HEAD
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
=======
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
>>>>>>> origin/dev
{
    AckPacket ack_packet(m_received_block_id);
    std::vector<BYTE> ack_buffer = ack_packet.ToBigEndianVector();
    const ssize_t size = ack_buffer.size();

    ssize_t bytes_written = m_socket->WriteDatagram(ack_buffer, host, port);

    if (bytes_written != size) {
<<<<<<< HEAD
            return std::make_pair(Status::kWriteError, bytes_written);
    }
    return std::make_pair(Status::kSuccess, bytes_written);
}

TFTPClient::Result TFTPClient::Read()
{
    
    ssize_t received_bytes = 
        m_socket->ReadDatagram(m_buffer, m_remote_addr, &m_remote_port);
    if (received_bytes == -1) {
        LOG(ERROR) << "No data received";
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

    LOG(DEBUG) << std::format("Buffer received: {} {} {} {}",
                            (int)m_buffer[0],
                            (int)m_buffer[1],
                            (int)m_buffer[2],
                            (int)m_buffer[3]);
    

    //add abstraction ?
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

TFTPClient::Result TFTPClient::GetFile(std::fstream &file)
=======
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
>>>>>>> origin/dev
{
    uint16_t totalReceivedBlocks = 0;
    uint16_t receivedDataBytes = 0;
    unsigned long totalReceivedDataBytes = 0;
<<<<<<< HEAD
    Result result;

    while (true) {
        // DATA
        result = this->Read();
        if (result.first != Status::kSuccess) {
            LOG(ERROR) << "GetFile(): Read error";
            return std::make_pair(result.first, totalReceivedDataBytes + std::max(result.second, 0));
        }
        receivedDataBytes = result.second;
        LOG(DEBUG) << std::format("receivedDataBytes: {}",receivedDataBytes);
        LOG(DEBUG) << std::format("m_received_block_id: {}",m_received_block_id);
        
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
            LOG(ERROR) << "GetFile(): SendAck failed";
            return std::make_pair(result.first, totalReceivedDataBytes);
        }

        LOG(INFO) << std::format("{} bytes ({} blocks) received", 
                                totalReceivedDataBytes,
                                totalReceivedBlocks);

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
        LOG(INFO) << std::format("{} bytes ({} blocks) written", 
                                total_written_bytes,
=======
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
        LOG(DEBUG) << std::format("GetData(): receivedDataBytes: {}", receivedDataBytes);
        LOG(DEBUG) << std::format("GetData(): m_received_block_id: {}", m_received_block_id);
        
        // Save data to buffer
        if ((receivedDataBytes > 0) && (m_received_block_id > totalReceivedBlocks)) {

            ++totalReceivedBlocks;
            totalReceivedDataBytes += receivedDataBytes;

            data.insert(data.end(),
                        buffer.begin() + kHeaderSize,
                        buffer.end());
        }

        // ACK
        status = SendAck(m_remote_addr.c_str(), m_remote_port);
        if (status != Status::kSuccess) {
            LOG(ERROR) << "GetData(): SendAck failed";
            return status;
        }

        LOG(DEBUG) << std::format("GetData(): {} bytes / {} blocks received", 
                                totalReceivedDataBytes,
                                totalReceivedBlocks);


    } while(receivedDataBytes == kHeaderSize + kDataMaxSize);
    
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
        LOG(DEBUG) << std::format("PutData(): {} bytes ({} blocks) written", 
                                written_bytes,
>>>>>>> origin/dev
                                current_block_id);
    }
}

<<<<<<< HEAD
uint8_t TFTPClient::GetHeaderSize()
=======


uint8_t TFTPClient::GetHeaderSize() 
>>>>>>> origin/dev
{
    return kHeaderSize; 
}

<<<<<<< HEAD
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
=======
uint16_t TFTPClient::GetMaxDataSize() 
{
    return kDataMaxSize; 
}


std::string  TFTPClient::GetDownloadedDefaultFName() 
{
    return kDownloadedDefaultFname;
}

std::string  TFTPClient::GetUploadedDefaultFName() 
{
    return kUploadedDefaultFname;
}
>>>>>>> origin/dev

