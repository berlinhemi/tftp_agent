#include "Executor.h"
#include "easylogging++.h"

#include <unistd.h>
#include <fcntl.h>
#include <chrono>
#include <sys/wait.h>

// Вспомогательная функция для перевода в нижний регистр
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
    
    // Защита от пустой команды
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
        // child proc
        close(stdoutPipe[0]);
        close(stderrPipe[0]);
        
        // redirect stdout/err to pipes
        dup2(stdoutPipe[1], STDOUT_FILENO);
        dup2(stderrPipe[1], STDERR_FILENO);
        
        // close original pipes
        close(stdoutPipe[1]);
        close(stderrPipe[1]);
        
        // Создаем новую группу процессов для дочернего процесса
        // Это позволит убивать все дочерние процессы вместе с родительским
        setpgid(0, 0);
        
        execl("/bin/sh", "sh", "-c", command.c_str(), NULL);
        
        exit(static_cast<int>(ExecStatus::ExeclFailed));
    } 
    else {
        // parent proc
        close(stdoutPipe[1]);
        close(stderrPipe[1]);
        
        // Устанавливаем неблокирующий режим для пайпов
        int flags = fcntl(stdoutPipe[0], F_GETFL, 0);
        fcntl(stdoutPipe[0], F_SETFL, flags | O_NONBLOCK);
        fcntl(stderrPipe[0], F_SETFL, flags | O_NONBLOCK);
        
        char buffer[4096];
        auto startTime = std::chrono::steady_clock::now();
        bool processExited = false;
        int status = 0;
        bool timeoutOccurred = false;
        
        while (!processExited && !timeoutOccurred) {
            // Проверяем таймаут
            auto now = std::chrono::steady_clock::now();
            auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - startTime).count();
            
            if (elapsed >= timeoutSeconds) {
                std::cout << "Timeout reached (" << timeoutSeconds << "s), killing process group " << pid << std::endl;
                
                // Убиваем всю группу процессов (отрицательный PID = группа процессов)
                // Сначала SIGTERM
                kill(-pid, SIGTERM);
                usleep(200000); // 200ms
                
                // Затем SIGKILL для гарантии
                kill(-pid, SIGKILL);
                usleep(100000); // 100ms
                
                // Проверяем, завершился ли процесс
                pid_t waitResult = waitpid(pid, &status, WNOHANG);
                if (waitResult == 0) {
                    // Если не завершился, ждем принудительно
                    waitpid(pid, &status, 0);
                }
                
                timeoutOccurred = true;
                result.exitCode = ExecStatus::Timeout;
                break;
            }
            
            // Читаем доступные данные из пайпов (неблокирующее)
            ssize_t count;
            
            // Чтение stdout
            count = read(stdoutPipe[0], buffer, sizeof(buffer) - 1);
            if (count > 0) {
                buffer[count] = '\0';
                result.output += buffer;
            } else if (count == -1 && errno != EAGAIN && errno != EWOULDBLOCK) {
                // Реальная ошибка чтения
                break;
            }
                        
            // Чтение stderr 
            count = read(stderrPipe[0], buffer, sizeof(buffer) - 1);
            if (count > 0) {
                buffer[count] = '\0';
                // Сохраняем ВСЁ, что пришло в stderr
                result.error += buffer;
            } else if (count == -1 && errno != EAGAIN && errno != EWOULDBLOCK) {
                break;
            }
            
            // Проверяем статус процесса (неблокирующее)
            pid_t waitResult = waitpid(pid, &status, WNOHANG);
            
            if (waitResult == pid) {
                // Процесс завершился
                processExited = true;
                break;
            } else if (waitResult == -1 && errno != ECHILD) {
                // Ошибка waitpid
                result.exitCode = ExecStatus::WaitFailed;
                break;
            }
            
            usleep(50000); // 50ms
        }
        
        // Если был таймаут, убеждаемся что процесс действительно завершен
        if (timeoutOccurred) {
            // Даем время на завершение
            usleep(300000); // 300ms
            
            // Финальная проверка
            pid_t waitResult = waitpid(pid, &status, WNOHANG);
            if (waitResult == 0) {
                // Процесс все еще жив - убиваем еще раз всю группу
                std::cout << "Process still alive, killing with SIGKILL again" << std::endl;
                kill(-pid, SIGKILL);
                waitpid(pid, &status, 0);
            } else if (waitResult == pid) {
                processExited = true;
            }
        }
        
        // Дочитываем остатки данных после завершения процесса
        fcntl(stdoutPipe[0], F_SETFL, flags);
        fcntl(stderrPipe[0], F_SETFL, flags);
        
        // Финальное чтение stdout
        ssize_t count;
        while ((count = read(stdoutPipe[0], buffer, sizeof(buffer) - 1)) > 0) {
            buffer[count] = '\0';
            result.output += buffer;
        }
        
        // Финальное чтение stderr
        while ((count = read(stderrPipe[0], buffer, sizeof(buffer) - 1)) > 0) {
            buffer[count] = '\0';
            std::string lower = ToLower(std::string(buffer));
           
            result.error += buffer;
            
        }
        
        close(stdoutPipe[0]);
        close(stderrPipe[0]);
        
        // Если процесс еще не завершился и не было таймаута, ждем его
        if (!processExited && !timeoutOccurred) {
            waitpid(pid, &status, 0);
        }
        
        // Обработка статуса завершения
        if (timeoutOccurred) {
            result.exitCode = ExecStatus::Timeout;
            // Проверяем, был ли процесс убит сигналом
            if (WIFSIGNALED(status)) {
                int signal_num = WTERMSIG(status);
                std::cout << "Process was killed by signal: " << signal_num << std::endl;
                if (signal_num == SIGTERM || signal_num == SIGKILL) {
                    result.exitCode = ExecStatus::SigTerminated;
                }
            }
        } else if (WIFEXITED(status)) {
            int exit_code = WEXITSTATUS(status);
            std::cout << "WIFEXITED OK:" << exit_code << "\n";
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

        return result;
    }
}

