#include "Executor.h"

#include <unistd.h>
#include <sys/wait.h>

bool Executor::Execute(const std::vector<BYTE>& command)
{
    std::string command_str (command.begin(), command.end());
    CommandResult result;
    
    int stdoutPipe[2];
    int stderrPipe[2];
    pipe(stdoutPipe);
    pipe(stderrPipe);
    
    pid_t pid = fork();
    
    if (pid == 0) {
        // Дочерний процесс
        close(stdoutPipe[0]);
        close(stderrPipe[0]);
        
        // Перенаправляем stdout и stderr
        dup2(stdoutPipe[1], STDOUT_FILENO);
        dup2(stderrPipe[1], STDERR_FILENO);
        
        close(stdoutPipe[1]);
        close(stderrPipe[1]);
        
        // Выполняем команду через shell
        execl("/bin/sh", "sh", "-c", command_str.c_str(), NULL);
        exit(EXIT_FAILURE);
    } else {
        // Родительский процесс
        close(stdoutPipe[1]);
        close(stderrPipe[1]);
        
        char buffer[256];
        ssize_t count;
        
        // Читаем stdout
        while ((count = read(stdoutPipe[0], buffer, sizeof(buffer) - 1)) > 0) {
            buffer[count] = '\0';
            result.output += buffer;
        }
        
        // Читаем stderr
        while ((count = read(stderrPipe[0], buffer, sizeof(buffer) - 1)) > 0) {
            buffer[count] = '\0';
            result.error += buffer;
        }
        
        close(stdoutPipe[0]);
        close(stderrPipe[0]);
        
        // Ждем завершения дочернего процесса
        int status;
        waitpid(pid, &status, 0);
        result.exitCode = WEXITSTATUS(status);
    }
    
    return result;
}