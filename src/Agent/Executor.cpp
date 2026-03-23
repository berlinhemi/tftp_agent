#include "Executor.h"
#include "easylogging++.h"

#include <unistd.h>
#include <fcntl.h>
#include <chrono>
#include <sys/wait.h>

// Helper function to convert to lowercase
std::string ToLower(const std::string& str) {
    std::string result;
    result.resize(str.length());
    for (size_t i = 0; i < str.length(); i++) {
        result[i] = tolower(str[i]);
    }
    return result;
}

CommandResult Executor::Execute(const std::string& command, int timeoutSeconds)
{
    CommandResult result;
    
    // Protection against empty command
    if (command.empty()) {
        result.exitCode = ExecStatus::Success;
        return result;
    }
    
    int stdoutPipe[2];
    int stderrPipe[2];
    if(pipe(stdoutPipe) == -1 ||  pipe(stderrPipe) == -1)
    {
        result.exitCode =  ExecStatus::PipeFailed;
        return result;
    }
   
    pid_t pid = fork();
    
    if (pid == -1) {
        close(stdoutPipe[0]); 
        close(stdoutPipe[1]);
        close(stderrPipe[0]);
        close(stderrPipe[1]);
        result.exitCode =  ExecStatus::ForkFailed;
        return result;
    }

    if (pid == 0) {
        // child process
        close(stdoutPipe[0]);
        close(stderrPipe[0]);
        
        // redirect stdout/err to pipes
        dup2(stdoutPipe[1], STDOUT_FILENO);
        dup2(stderrPipe[1], STDERR_FILENO);
        
        // close original pipes
        close(stdoutPipe[1]);
        close(stderrPipe[1]);
        
        // Create a new process group for the child process
        // This allows killing all child processes together with the parent
        setpgid(0, 0);
        
        execl("/bin/sh", "sh", "-c", command.c_str(), NULL);
        
        exit(static_cast<int>(ExecStatus::ExeclFailed));
    } 
    else {
        // parent process
        close(stdoutPipe[1]);
        close(stderrPipe[1]);
        
        // Set non-blocking mode for pipes
        int flags = fcntl(stdoutPipe[0], F_GETFL, 0);
        fcntl(stdoutPipe[0], F_SETFL, flags | O_NONBLOCK);
        fcntl(stderrPipe[0], F_SETFL, flags | O_NONBLOCK);
        
        char buffer[4096];
        auto startTime = std::chrono::steady_clock::now();
        bool processExited = false;
        int status = 0;
        bool timeoutOccurred = false;
        
        while (!processExited && !timeoutOccurred) {
            // Check timeout
            auto now = std::chrono::steady_clock::now();
            auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - startTime).count();
            
            if (elapsed >= timeoutSeconds) {
                std::cout << "Timeout reached" << std::endl;
                // Give a chance to terminate gracefully
                kill(-pid, SIGTERM);
                usleep(200000);
                pid_t waitResult = waitpid(pid, &status, WNOHANG);
                // Kill proccess group
                if (waitResult == 0) {
                    kill(-pid, SIGKILL);
                    // blocking call
                    waitpid(pid, &status, 0);  
                }
                timeoutOccurred = true;
                processExited = true;
                result.exitCode = ExecStatus::Timeout;
                break;
            }
            
            // Read available data from pipes (non-blocking)
            ssize_t count;
            
            // Read stdout
            count = read(stdoutPipe[0], buffer, sizeof(buffer) - 1);
            if (count > 0) {
                buffer[count] = '\0';
                result.output += buffer;
            } else if (count == -1 && errno != EAGAIN) {
                // Actual read error
                break;
            }
                        
            // Read stderr
            count = read(stderrPipe[0], buffer, sizeof(buffer) - 1);
            if (count > 0) {
                buffer[count] = '\0';
                // Save EVERYTHING that came to stderr
                result.error += buffer;
            } else if (count == -1 && errno != EAGAIN) {
                break;
            }
            
            // Check process status (non-blocking)
            pid_t waitResult = waitpid(pid, &status, WNOHANG);
            
            if (waitResult == pid) {
                // Process terminated
                processExited = true;
                break;
            } else if (waitResult == -1 && errno != ECHILD) {
                // waitpid error
                result.exitCode = ExecStatus::WaitFailed;
                break;
            }
            
            usleep(50000); // 50ms
        }
        
        
        // Wait remaining data after process termination
        fcntl(stdoutPipe[0], F_SETFL, flags);
        fcntl(stderrPipe[0], F_SETFL, flags);
        
        // Final read of stdout
        ssize_t count;
        while ((count = read(stdoutPipe[0], buffer, sizeof(buffer) - 1)) > 0) {
            buffer[count] = '\0';
            result.output += buffer;
        }
        
        // Final read of stderr
        while ((count = read(stderrPipe[0], buffer, sizeof(buffer) - 1)) > 0) {
            buffer[count] = '\0';
            std::string lower = ToLower(std::string(buffer));
            result.error += buffer;
            
        }
        
        close(stdoutPipe[0]);
        close(stderrPipe[0]);
        
        // If no timeout occurred => get exit status
        if (!timeoutOccurred) {
            waitpid(pid, &status, 0);
            
            if (WIFEXITED(status)) {
                int exit_code = WEXITSTATUS(status);
                result.exitCode = static_cast<ExecStatus>(exit_code);
            } 
            else if (WIFSIGNALED(status)) {
                int signal_num = WTERMSIG(status);
                std::cout << "Process terminated by signal: " << signal_num << std::endl;
                result.exitCode = ExecStatus::ChildSignaled;
            }
            else if (WIFSTOPPED(status)) {
                result.exitCode = ExecStatus::ChildStopped;
            }
            else {
                result.exitCode = ExecStatus::UnknownError;
            }
        }

        return result;
    }
}