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
    if (key_data.empty()) {
        LOG(ERROR) << "SetKey failed: empty key not allowed";
        return false;
    }

    m_key.assign(key_data.begin(), key_data.end());
    return true;
}

std::vector<BYTE> Packer::Pack(const std::vector<BYTE>& buffer)
{
    
    if (buffer.empty()) 
        return {};
    if(buffer.size() > kMaxDataSizeBytes){
        LOG(ERROR) << "Packer: buffer size exceeds max size:" << kMaxDataSizeBytes << " bytes";
        return {};
    }
    std::vector<BYTE> compressed = Compress(buffer);
    return Encrypt(compressed);
}


std::vector<BYTE> Packer::Unpack(const std::vector<BYTE>& buffer)
{
    if (buffer.empty())
        return {};
    std::vector<BYTE> decpypted = Decrypt(buffer);
    return Decompress(decpypted);
}


std::vector<BYTE> Packer::Compress(const std::vector<BYTE>& buffer)
{
    uLong compressedSize = compressBound(buffer.size());
    std::vector<BYTE> compressed(compressedSize);

    int result = compress(compressed.data(), &compressedSize,
                            buffer.data(), buffer.size());

    if (result != Z_OK) {
        LOG(ERROR) << "compress() failed with code " + std::to_string(result);
        return {};
    }

    // Resize to actual compressed size
    compressed.resize(compressedSize);
    return compressed;
}


std::vector<BYTE> Packer::Decompress(const std::vector<BYTE>& buffer)
{
    std::vector<BYTE> decompressed(kMaxDataSizeBytes);
    uLong destLen = kMaxDataSizeBytes;
    int result = uncompress(decompressed.data(), &destLen,
                            buffer.data(), buffer.size());
    
    if (result != Z_OK) {
        LOG(ERROR) << "uncompress() failed with code " + std::to_string(result);
        return {};
    }

    // Resize to actual decompressed size
    decompressed.resize(destLen); 
    return decompressed;
}


std::vector<BYTE> Packer::Encrypt(const std::vector<BYTE>& buffer)
{
    if (m_key.empty()) {
        LOG(ERROR) << "Encryption failed: key not set";
        return {};  
    }

    RC4_KEY rc4_key;
    RC4_set_key(&rc4_key, m_key.size(), m_key.data());

    std::vector<unsigned char> output(buffer.size());
    RC4(&rc4_key, buffer.size(), buffer.data(), output.data());

    return output;
}

std::vector<BYTE> Packer::Decrypt(const std::vector<BYTE>& buffer)
{
    if(m_key.empty()){
        LOG(ERROR) << "Decryption failed: key not set";
        return {};  
    }

    return Encrypt(buffer);
}
