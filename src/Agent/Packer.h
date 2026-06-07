#ifndef AGENT_PACKER_H
#define AGENT_PACKER_H

#include <vector>
#include <string>

typedef unsigned char BYTE;

/**
 * @brief Data packer with compression (zlib) and encryption (RC4).
 * 
 * Provides compression and encryption chain: Compress → Encrypt for Pack(),
 * and Decrypt → Decompress for Unpack(). Uses RC4 stream cipher and zlib.
 * 
 * @note Maximum data size: kMaxDataSizeBytes (100 MB)
 */
class Packer
{
public:
    /**
     * @brief Construct packer with encryption key.
     * @param key_data Encryption key (empty key allowed but will fail on encrypt/decrypt)
     */
    Packer(const std::string& key_data);
    /**
     * @brief Set or update encryption key.
     * @param key_data Non-empty encryption key
     * @return true if key set successfully, false if key is empty
     */
    bool SetKey(const std::string& key_data);
    /**
     * @brief Compress and encrypt data.
     * @param buffer Input data
     * @return Packed data, or empty vector on error (empty input, exceeds size limit, or operation failed)
     */
    std::vector<BYTE> Pack(const std::vector<BYTE>& buffer);
     /**
     * @brief Decrypt and decompress data.
     * @param buffer Packed data
     * @return Original data, or empty vector on error (empty input or operation failed)
     */
    std::vector<BYTE> Unpack(const std::vector<BYTE>& buffer);
    
    static const inline size_t kMaxDataSizeBytes = 100*1024*1024; ///< Maximum allowed data size (100 MB)
private:

    std::vector<BYTE> m_key;
    std::vector<BYTE> Compress(const std::vector<BYTE>& buffer);
    std::vector<BYTE> Decompress(const std::vector<BYTE>& buffer);
    std::vector<BYTE> Encrypt(const std::vector<BYTE>& buffer);
    std::vector<BYTE> Decrypt(const std::vector<BYTE>& buffer);
};


#endif // AGENT_PACKER_H