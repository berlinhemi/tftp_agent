

#include "ArgParser.h"
#include "TFTPClient.h"

#include <algorithm>
#include <chrono>
#include <format>
#include <sstream>

#include "easylogging++.h"
// Define it only once in project
INITIALIZE_EASYLOGGINGPP 


void ShowHelp(const std::string program_name)
{
    std::cout << format("\nTFTP Agent\n"
        "Usage:   {0} -h HOST -o OPERATION [-f FNAME] \n"  
        "\t-h: hostname\n"
        "\t-o: operation\n"
        "\t-f: filename (optional). Default filename for get is 'input', for put is 'output'\n"
        "Examples:\n"
        "\t{0} -h 192.168.1.104 -o get \n"
        "\t{0} -h server.com -o put -f result\n"
        , program_name);
    
}

int main(int argc, char **argv)
{
    //el::Configurations conf;
    //conf.setToDefault();
    //conf.setGlobally(el::
    //el::Configurations conf("/path/to/my-conf.conf");
    // Usage: 192.168.1.104 get example.txt


    //el::Loggers::reconfigureAllLoggers(el::ConfigurationType::ToFile, "false");
    //el::Loggers::reconfigureAllLoggers(el::ConfigurationType::Format, "[%level] %msg");
   

    // el::Logger* defaultLogger = el::Loggers::getLogger("default");
    
    // // Set the logging level
    // // You can change this to the desired level
    // defaultLogger->configure(el::Level::Info, el::ConfigurationType::Enabled, "true");
    
    // // Disable all levels below Info (this will disable Debug)
    // defaultLogger->configure(el::Level::Debug, el::ConfigurationType::Enabled, "false");
    
    el::Configurations defaul_conf;
    defaul_conf.setToDefault();
    defaul_conf.set(el::Level::Info, el::ConfigurationType::Enabled, "true");
    defaul_conf.set(el::Level::Debug, el::ConfigurationType::Enabled, "false");
    
    el::Loggers::reconfigureLogger("default", defaul_conf);
    el::Loggers::reconfigureLogger("default", el::ConfigurationType::Format, "[%level] %msg");
    el::Loggers::reconfigureLogger("default", el::ConfigurationType::ToFile, "false");


    // LOG(INFO) << "INFO enabled";
    // LOG(ERROR) << "ERROR enabled";
    // LOG(DEBUG) << "DEBUG enabled";
    
    ArgParser arg_parser(argc, argv);
    if(!arg_parser.cmdOptionExists("-h"))
    {
        LOG(ERROR) << "Option -h not found";
        ShowHelp(argv[0]);
        return -1;
    }
    if(!arg_parser.cmdOptionExists("-o"))
    {
        LOG(ERROR) << "Option -o not found";
        ShowHelp(argv[0]);
        return -1;
    }

    const uint16_t port = 69;
    std::string host = arg_parser.getCmdOption("-h");
    std::string operation = arg_parser.getCmdOption("-o");
    TFTPClient::RequestType request_type;
    if(operation == "get")
    {
        request_type = TFTPClient::RequestType::GET;
    }
    else if (operation == "put")
    {
        request_type = TFTPClient::RequestType::PUT;
    }
    else
    {
        LOG(ERROR) << "Unknow operation";
        ShowHelp(argv[0]);
        return -1;
    }

    std::string fname;
    if(arg_parser.cmdOptionExists("-f"))
    {
        fname = arg_parser.getCmdOption("-f");
    }
    else
    {
        LOG(INFO) << "Using default file name";
        fname = (operation == "get") ?
         TFTPClient::GetDownloadedDefaultFName() 
         : TFTPClient::GetUploadedDefaultFName();
    }
    

    LOG(INFO) << std::format("Start TFTP Client {}:{}", host.c_str(), port);

   
    UdpSocket sock;
    TFTPClient client(&sock, host, port);

    std::vector<BYTE> command;
    //const auto begin = std::chrono::steady_clock::now();
    TFTPClient::Status status;
    if (request_type == TFTPClient::RequestType::GET)
    {
        status = client.Get(command, fname); 
        std::ostringstream oss;
        LOG(INFO) << "Command size:" << command.size();
        oss << "GetCommand result: ";
        for (auto e : command){
            oss << e << " ";
        }
        LOG(INFO) << oss.str();
    }
    else if (request_type == TFTPClient::RequestType::PUT)
    {
        // Test data
        std::vector<BYTE> data(2000, '1');
        status = client.Put(data, fname);
    }
  
    
    //const auto end = std::chrono::steady_clock::now();

    if (status != TFTPClient::Status::kSuccess) {
        LOG(ERROR) << client.ErrorDescription(status).c_str();
        return 1;
    }

  

    //LOG(INFO) << std::format("Elapsed time: {} [ms]", 
    //                       std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count());
  
    return 0;
}
