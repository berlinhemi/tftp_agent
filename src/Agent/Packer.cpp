#include "Packer.h"

#include "easylogging++.h"
#include <openssl/rc4.h>
#include <stdexcept>
#include "zlib.h"

Packer::Packer(const std::string& key_data)
{
    m_key.assign(key_data.begin(), key_data.end());
}

bool Packer::SetKey(const std::string& key_data)
{
    if(key_data.empty()){
        return false;
    }

    m_key.assign(key_data.begin(), key_data.end());
    return true;
}

std::vector<BYTE> Packer::Pack(const std::vector<BYTE>& buffer)
{
    try {
        if (buffer.empty()) 
            return {};
        if(buffer.size() > MAX_DATA_SIZE){
            LOG(ERROR) << "Packer: buffer size exceeds " << MAX_DATA_SIZE << " bytes";
            return {};
        }
        std::vector<BYTE> compressed = Compress(buffer);
        return Encrypt(compressed);
    }
    catch (const std::exception& e) {
        LOG(ERROR) << "Packer: packing failed. Details: " << e.what();
        return {}; 
    }
}


std::vector<BYTE> Packer::Unpack(const std::vector<BYTE>& buffer)
{
    try {
        if (buffer.empty())
            return {};
        std::vector<BYTE> decpypted = Decrypt(buffer);
        return Decompress(decpypted);
    }
    catch (const std::exception& e) {
        LOG(ERROR) << "Packer: unpacking failed. Details: " << e.what();
        return {}; 
    }
}


std::vector<BYTE> Packer::Compress(const std::vector<BYTE>& buffer)
{
    uLong compressedSize = compressBound(buffer.size());
    std::vector<BYTE> compressed(compressedSize);

    int result = compress(compressed.data(), &compressedSize,
                            buffer.data(), buffer.size());

    if (result != Z_OK) {
        throw std::runtime_error("compress() failed with code " + std::to_string(result));
    }

    // Resize to actual compressed size
    compressed.resize(compressedSize);
    return compressed;
}


std::vector<BYTE> Packer::Decompress(const std::vector<BYTE>& buffer)
{
    std::vector<BYTE> decompressed(MAX_DATA_SIZE);
    uLong destLen = MAX_DATA_SIZE;
    int result = uncompress(decompressed.data(), &destLen,
                            buffer.data(), buffer.size());
    
    if (result != Z_OK) {
        throw std::runtime_error("uncompress() failed with code " + std::to_string(result));
    }

    // Resize to actual decompressed size
    decompressed.resize(destLen); 
    return decompressed;
}


std::vector<BYTE> Packer::Encrypt(const std::vector<BYTE>& buffer)
{

    if(m_key.empty()){
        throw std::runtime_error("Encryption failed: key not set");
    }

    // Initialize RC4 key schedule
    RC4_KEY rc4_key;
    RC4_set_key(&rc4_key, m_key.size(), m_key.data());


    std::vector<unsigned char> output(buffer.size());
    RC4(&rc4_key, buffer.size(), buffer.data(), output.data());

    return output;
}

std::vector<BYTE> Packer::Decrypt(const std::vector<BYTE>& buffer)
{
    if(m_key.empty()){
        throw std::runtime_error("Decryption failed: key not set");
    }

    return Encrypt(buffer);
}
