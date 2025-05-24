#ifndef TFTPPACKET_H
#define TFTPPACKET_H

#include <cstring>

#include <stdexcept>
#include <string>
#include <vector>

typedef unsigned char BYTE;

enum class OpCode {
        RRQ = 1, WRQ, DATA, ACK, ERR, OACK
    };

class DataPacket
{
public:
    //const static uint16_t kMaxSize = 516;
    
    DataPacket() = default;


    DataPacket(uint16_t block_id, const std::vector<BYTE>& payload ): block_id(block_id)
    {
        if(payload.size() > kMaxDataSize)
        {
            throw std::invalid_argument("Too big payload buffer.");
        }
        data.assign(payload.begin(), payload.end());
    }

    bool InitFromBigEndianVector(std::vector<BYTE> buffer)
    {
        if(buffer.size() > kMaxDataSize + kMaxDataSize)
        {
            return false;
        }
        block_id = (buffer[2] << 8) | buffer[3] ;
        data.assign(buffer.begin() + kHeaderSize, buffer.end());
        return true;
    }

    std::vector<BYTE> ToBigEndianVector()
    {
        std::vector<BYTE> buffer;
        buffer.push_back(0);
        buffer.push_back(m_opcode);
        buffer.push_back((uint16_t)(block_id >> 8));
        buffer.push_back((uint16_t)(block_id & 0x00FF));

        std::copy(data.begin(), data.end(), std::back_inserter(buffer));
        return buffer;
    }

    uint16_t PacketSize()
    {
        return static_cast<uint16_t>(data.size()) + kHeaderSize;
    }
private:
    static const uint16_t kMaxDataSize = 512;
    static const uint16_t kHeaderSize = 4;
    static const uint16_t m_opcode = (uint16_t)OpCode::DATA;
    uint16_t block_id;
    std::vector<BYTE> data;
};

class RequestPacket
{
public:
   
    RequestPacket(OpCode opcode, std::string fname, std::string type ) :
        m_opcode((uint16_t)opcode),
        m_fname(fname),
        m_type(type)
    {}

    std::vector<BYTE> ToBigEndianVector()
    {
        std::vector<BYTE> data;
        data.push_back(0);
        data.push_back(m_opcode);
        std::copy(m_fname.begin(), m_fname.end(), std::back_inserter(data));
        data.push_back(0);
        std::copy(m_type.begin(), m_type.end(), std::back_inserter(data));
        data.push_back(0);
        return data;
    }
private:
    uint16_t m_opcode;
    std::string m_fname;
    std::string m_type;
};

class AckPacket
{
public:
    AckPacket() = default;

    AckPacket(uint16_t block_id) : m_block_id(block_id)
    {}

    std::vector<BYTE> ToBigEndianVector()
    {
        std::vector<BYTE> data;
        data.push_back(0);
        data.push_back(m_opcode);
        data.push_back((uint16_t)(m_block_id >> 8));
        data.push_back((uint16_t)(m_block_id & 0x00FF));
        return data;
    }
private:
    static const uint16_t m_opcode = (uint16_t)OpCode::ACK;
    uint16_t m_block_id;
};


#endif