#ifndef EXECUTOR_H
#define EXECUTOR_H

#include <optional>
#include <string>
#include <vector>

typedef unsigned char BYTE;
// static const size_t MAX_DATA_SIZE = 10*1024*1024; // 10 MB

enum class ExecStatus {
    Success = 0,
    CommandNotFound = 127,
    PermissionDenied = 126,
    
    // Custom codes
    ForkFailed = 200,
    PipeFailed = 201,
    WaitFailed = 202,
    ExeclFailed = 203,
    ChildSignaled = 204,
    ChildStopped = 205,
    
    // General errors
    UnknownError = 300
};

struct CommandResult {
    std::string output;
    std::string error;
    ExecStatus exitCode;
    //bool timedOut; //TODO
};

class Executor
{
public:
    static CommandResult Execute(const std::string& command); 
//private:
//    static const int EXECL_FAILURE = 150;
};


#endif // EXECUTOR_H