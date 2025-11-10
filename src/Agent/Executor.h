#ifndef EXECUTOR_H
#define EXECUTOR_H

#include <vector>
#include <string>

typedef unsigned char BYTE;
// static const size_t MAX_DATA_SIZE = 10*1024*1024; // 10 MB

class Executor
{
public:
    Executor();
    bool Execute(const std::vector<BYTE>& command); // better string, then someone convert to bytes if need it 
   
private:

    struct CommandResult {
        std::string output;
        std::string error;
        int exitCode;
    };
    std::vector<BYTE> stdout_buff;
    std::vector<BYTE> stderr_buff;
};


#endif // EXECUTOR_H