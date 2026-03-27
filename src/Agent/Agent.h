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
    std::string GetTask();
    void DoIteration();
    void SendResult(CommandResult result);
private:
    std::string host;
    uint16_t port;
    std::string encryptionKey;
};











#endif