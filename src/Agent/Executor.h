#ifndef EXECUTOR_H
#define EXECUTOR_H

#include <optional>
#include <string>
#include <vector>

typedef unsigned char BYTE;

/**
 * @brief Shell command executor with timeout, stdout/stderr capture.
 * Executes via /bin/sh -c with configurable timeout (default: 600s).
 * Uses process groups to kill entire process tree on timeout.
 */
enum class ExecStatus {
    Success = 0,
    CommandNotFound = 127,
    PermissionDenied = 126,
    SigTerminated = 143,
    
    // Custom codes
    ForkFailed = 200,
    PipeFailed = 201,
    WaitFailed = 202,
    ExeclFailed = 203,
    ChildSignaled = 204,
    ChildStopped = 205,
    Timeout = 206,
    
    // General errors
    UnknownError = 300
};

struct CommandResult {
    std::string std_out;
    std::string std_err;
    ExecStatus exitCode;
};

class Executor
{
public:
    /**
     * Execute shell command with timeout.
     * @param command Shell command to execute
     * @param timeoutSeconds Max execution time (default: 600)
     * @return CommandResult with output and exit status
     */
    static CommandResult Execute(const std::string& command,  int timeoutSeconds = 600); 
    
    /**
     * Get human-readable description for status code.
     */
    static std::string ErrorDescription(ExecStatus execStatus);
};


#endif // EXECUTOR_H