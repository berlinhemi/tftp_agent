#ifndef EXECUTOR_H
#define EXECUTOR_H

#include <optional>
#include <string>
#include <vector>

typedef unsigned char BYTE;
// static const size_t MAX_DATA_SIZE = 10*1024*1024; // 10 MB

struct CommandResult {
    std::string output;
    std::string error;
    int exitCode;
    //bool timedOut; //TODO
};

class Executor
{
public:
    static std::optional<CommandResult> Execute(const std::string& command); 
private:
    static const int EXECL_FAILURE = 150;
};


#endif // EXECUTOR_H