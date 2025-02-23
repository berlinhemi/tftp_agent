#ifndef TFTPPACKET_H
#define TFTPPACKET_H

#include <cstring>

#include <stdexcept>
#include <string>
#include <vector>

enum class OpCode {
        RRQ = 1, WRQ, DATA, ACK, ERR, OACK
    };

class DataPacket
{
public:
    const static uint16_t kMaxSize = 516;
    
    DataPacket()
    {}

    DataPacket(uint16_t block_id, const std::vector<BYTE>& buffer ):
        block_id(block_id)
    {
        if(buffer.size() > kMaxDataSize)
        {
            throw std::length_error("Too big data chunk.");
        }
        data.assign(buffer.begin(), buffer.end());
    }

    void InitFromBigEndianVector(std::vector<BYTE> buffer)
    {
        block_id = (buffer[2] << 8) | buffer[3] ;
        data.assign(buffer.begin() + kHeaderSize, buffer.end());
    }

    std::vector<BYTE> ToBigEndianVector()
    {
        std::vector<BYTE> buffer;
        buffer.push_back(0);
        buffer.push_back(opcode);
        buffer.push_back((uint16_t)(block_id >> 8));
        buffer.push_back((uint16_t)(block_id & 0x00FF));
        std::copy(data.begin(), data.end(), std::back_inserter(data));
        return buffer;
    }
private:
    static const uint16_t kMaxDataSize = 512;
    static const uint16_t kHeaderSize = 4;
    uint16_t opcode = (uint16_t)OpCode::DATA;
    uint16_t block_id;
    std::vector<BYTE> data;
};

class RequestPacket
{
public:
   
    RequestPacket(OpCode opcode, std::string fname, std::string type ):
        opcode((uint16_t)opcode), fname(fname), type(type)
    {}

    std::vector<BYTE> ToBigEndianVector()
    {
        std::vector<BYTE> data;
        data.push_back(0);
        data.push_back(opcode);
        std::copy(fname.begin(), fname.end(), std::back_inserter(data));
        data.push_back(0);
        std::copy(type.begin(), type.end(), std::back_inserter(data));
        data.push_back(0);
        return data;
    }
private:
    uint16_t opcode;
    std::string fname;
    std::string type;
};

class AckPacket
{
public:
    AckPacket()
    {}

    AckPacket(uint16_t block_id):
        block_id(block_id)
    {}

    std::vector<BYTE> ToBigEndianVector()
    {
        std::vector<BYTE> data;
        data.push_back(0);
        data.push_back(opcode);
        data.push_back((uint16_t)(block_id >> 8));
        data.push_back((uint16_t)(block_id & 0x00FF));
        return data;
    }
private:
    uint16_t opcode = (uint16_t)OpCode::ACK;
    uint16_t block_id;
};


#endif