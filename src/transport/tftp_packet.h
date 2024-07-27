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
    RequestPacket(OpCode opcode,std::string fname,std::string type ):
        opcode((uint16_t)opcode), fname(fname), type(type)
    {}

    std::vector<BYTE> ToVector ()
    {
        size_t offset = 0;
        std::vector<BYTE> data;
        //data.reserve(sizeof(opcode) + fname.size() + 1 + type.size() + 1);
        
        data.push_back(0);
        data.push_back(opcode);
        //offset += 2;
        std::copy(fname.begin(), fname.end(), std::back_inserter(data));
        data.push_back(0);
        
        //offset += fname.size();
        //offset++;
        std::copy(type.begin(), type.end(), std::back_inserter(data));
        data.push_back(0);
        //std::strcpy((char*)&data[offset], type.c_str());

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
};


#endif