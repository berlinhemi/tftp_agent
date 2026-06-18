#ifndef AGENT_H
#define AGENT_H

#include "Executor.h"

#include <cstdint>
#include <string>
#include <vector>

typedef unsigned char BYTE;

/**
 * @brief TFTP-based agent that executes commands and returns results.
 * 
 * Implements a simple agent pattern where:
 * 1. Downloads encrypted command from TFTP server
 * 2. Decrypts and executes the command
 * 3. Encrypts and uploads execution results back to server
 * 
 * @details Workflow:
 * - Downloads file "input" from TFTP server (default filename)
 * - Unpacks (decrypts + decompresses) the command
 * - Executes command via Executor with 600s timeout
 * - Packs (compresses + encrypts) stdout/stderr
 * - Uploads result with unique filename to TFTP server
 * 
 * @note Uses TFTP client for file transfer
 * @note Uses Packer for encryption (RC4) and compression (zlib)
 * @note Default TFTP port is 69
 */
class Agent
{
public:
    /**
     * @brief Constructs Agent with server connection parameters.
     * 
     * @param host TFTP server IP address (dotted-decimal format)
     * @param encryptionKey RC4 encryption key for packing/unpacking
     * @param port TFTP server port (default: 69)
     * 
     * @note Key must be non-empty for encryption/decryption to work
     */
    Agent(std::string host,  std::string encryptionKey, uint16_t port = 69);

    /**
     * @brief Performs one complete agent iteration: get command, execute, send result.
     * 
     * This is the main workflow method that:
     * - Calls GetCommand() to retrieve and decrypt command
     * - Executes command via Executor::Execute()
     * - Calls SendResult() to upload encrypted results
     * 
     * @note If command is empty, iteration stops without execution
     * @note Errors are logged but method doesn't throw exceptions
     */
    void DoIteration();
    
private:
    std::string GetCommand();
    bool SendResult(CommandResult result);

    std::string m_host;         ///< TFTP server IP address
    uint16_t m_port;            ///< TFTP server port (default: 69)
    std::string m_encryptionKey;///< RC4 encryption key for packing/unpacking
};

#endif