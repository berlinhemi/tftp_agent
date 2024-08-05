#ifndef TFTPPACKET_H
#define TFTPPACKET_H

#include <cstring>
#include <string>
#include <vector>

enum class OpCode {
        RRQ = 1, WRQ, DATA, ACK, ERR, OACK
    };

struct RequestPacket
{
    uint16_t opcode;
    std::string fname;
    std::string type;
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
};

struct AckPacket
{
    uint16_t opcode = (uint16_t)OpCode::ACK;
    uint16_t block_id;
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
};


#endif