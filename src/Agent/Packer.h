#ifndef AGENT_PACKER_H
#define AGENT_PACKER_H

#include <vector>
#include <string>

typedef unsigned char BYTE;


class Packer
{
public:
    Packer(const std::string& key_data);
    bool SetKey(const std::string& key_data);
    std::vector<BYTE> Pack(const std::vector<BYTE>& buffer);
    std::vector<BYTE> Unpack(const std::vector<BYTE>& buffer);
    
    static const inline size_t kMaxDataSizeBytes = 100*1024*1024; // 100 MB
private:

    std::vector<BYTE> m_key;
    std::vector<BYTE> Compress(const std::vector<BYTE>& buffer);
    std::vector<BYTE> Decompress(const std::vector<BYTE>& buffer);
    std::vector<BYTE> Encrypt(const std::vector<BYTE>& buffer);
    std::vector<BYTE> Decrypt(const std::vector<BYTE>& buffer);
};


#endif // AGENT_PACKER_H