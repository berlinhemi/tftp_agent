#include "TFTPClient.h"
#include <algorithm>
#include <chrono>
#include <format>

#include "easylogging++.h"
// Define it only once in project
INITIALIZE_EASYLOGGINGPP 

void show_help(const std::string program_name)
{
    std::cout << format("nTFTP Client\n"
        "Usage:   {0} HOST [get | put] FILE\n"  
        "Examples:\n\t{0} 192.168.1.104 get somefile.txt\n"
        "\t{0} server.com put somefile.txt\n"
        , program_name);
    
}

int main(int argc, char **argv)
{
    //el::Configurations conf;
    //conf.setToDefault();
    //conf.setGlobally(el::
    //el::Configurations conf("/path/to/my-conf.conf");
    // Usage: 192.168.1.104 get example.txt

    el::Loggers::reconfigureAllLoggers(el::ConfigurationType::Format, "[%level] %msg");

    
    if (argc < 4) {
        show_help(argv[0]);
        return 1;
    }

    const uint16_t port = 69;

    std::string host(argv[1]);
    std::string operation(argv[2]);
    std::string filename(argv[3]);

    
    std::transform(operation.begin(), operation.end(), operation.begin(),
                   [](unsigned char c){ return std::tolower(c); });

    const int opCode = operation.compare("get") == 0 ? 1 : (operation.compare("put") == 0 ? 2 : 0);
    if (opCode == 0) {
        show_help(argv[0]);
        return 1;
    }

    LOG(INFO) << std::format("Start TFTP Client {}:{}", host.c_str(), port);

   
    UdpSocket sock;
    TFTPClient client(&sock, host, port);

    const auto begin = std::chrono::steady_clock::now();
    const TFTPClient::Status status = opCode == 1 ? client.Get(filename) : client.Put(filename);
    const auto end = std::chrono::steady_clock::now();

    if (status != TFTPClient::Status::kSuccess) {
        LOG(ERROR) << client.ErrorDescription(status).c_str();
        return 1;
    }

    LOG(INFO) << std::format("Elapsed time: {} [ms]", 
                            std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count());
  
    return 0;
}
