#ifndef AGENT_H
#define AGENT_H

#include "Executor.h"

#include <cstdint>
#include <string>
#include <vector>

typedef unsigned char BYTE;

class Agent
{
public:
    Agent(std::string host,  std::string encryptionKey, uint16_t port = 69);
    void DoIteration();
    
private:
    std::string GetCommand();
    bool SendResult(CommandResult result);

    std::string m_host;
    uint16_t m_port;
    std::string m_encryptionKey;

    
};





#endif