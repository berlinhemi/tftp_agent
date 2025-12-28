#include "Executor.h"

#include <unistd.h>
#include <sys/wait.h>


CommandResult Executor::Execute(const std::string& command)
{
    CommandResult result;
    
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
        // child proc
        close(stdoutPipe[0]);
        close(stderrPipe[0]);
        
        // redirect stdout/err to pipes
        dup2(stdoutPipe[1], STDOUT_FILENO);
        dup2(stderrPipe[1], STDERR_FILENO);
        
        // close orginal pipes
        close(stdoutPipe[1]);
        close(stderrPipe[1]);
        
        // change currect process to shell
        execl("/bin/sh", "sh", "-c", command.c_str(), NULL);
        exit(static_cast<int>(ExecStatus::ExeclFailed));
    } 
    else {
        // parent proc
        close(stdoutPipe[1]);
        close(stderrPipe[1]);
        
        char buffer[256];
        ssize_t count;
        
        // read stdout
        while ((count = read(stdoutPipe[0], buffer, sizeof(buffer) - 1)) > 0) {
            buffer[count] = '\0';
            result.output += buffer;
        }
        
        // read stderr
        while ((count = read(stderrPipe[0], buffer, sizeof(buffer) - 1)) > 0) {
            buffer[count] = '\0';
            result.error += buffer;
        }
        
        close(stdoutPipe[0]);
        close(stderrPipe[0]);
        
        int status;
        // WARNING: blocking waiting
        if (waitpid(pid, &status, 0) == -1) {
            result.exitCode =  ExecStatus::WaitFailed;
            return result;
        }

        if (WIFEXITED(status)) {
            int exit_code = WEXITSTATUS(status);
            result.exitCode = static_cast<ExecStatus>(exit_code);
            
        } 
        else if (WIFSIGNALED(status)) {
            // Killed by signal
            result.exitCode = ExecStatus::ChildSignaled;
            // if need signal id:
            // int signal_num = WTERMSIG(status);
        }
        else if (WIFSTOPPED(status)) {
            // Stopped (rarely)
            result.exitCode = ExecStatus::ChildStopped;
        }
        else {
            result.exitCode = ExecStatus::UnknownError;
        }

        return result;
    }
}