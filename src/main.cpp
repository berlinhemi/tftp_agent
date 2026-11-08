
#include "Agent/Agent.h"
#include "Agent/ArgParser.h"
#include "Agent/Packer.h"
#include "Transport/TFTPClient.h"

#include <algorithm>
#include <chrono>
#include <format>
#include <sstream>

#include "easylogging++.h"
#include "easylogging++.cc"

// Define it only once in project
INITIALIZE_EASYLOGGINGPP 

static void ShowHelp(const std::string program_name)
{
    std::cout << format("\nTFTP Agent\n"
        "Usage:   {0} -h SERVER -a ACTION [-f FNAME] \n"  
        "\t-s: hostname\n"
        "\t-a: action\n"
        "\t-f: filename (optional). Default filename for GET is 'input', for PUT is 'output'\n"
        "Examples:\n"
        "\t{0} -s 192.168.1.104 -a get \n"
        "\t{0} -s server.com -a put -f result\n"
        , program_name);
    
}

static void ConfigureLogger()
{
    el::Configurations defaul_conf;
    defaul_conf.setToDefault();
    defaul_conf.set(el::Level::Info, el::ConfigurationType::Enabled, "true");
    defaul_conf.set(el::Level::Debug, el::ConfigurationType::Enabled, "false");
    
    el::Loggers::reconfigureLogger("default", defaul_conf);
    el::Loggers::reconfigureLogger("default", el::ConfigurationType::Format, "[%level] %msg");
    el::Loggers::reconfigureLogger("default", el::ConfigurationType::ToFile, "false");
}


int main(int argc, char **argv)
{

    ConfigureLogger();
    if(argc == 1)
        ShowHelp(argv[0]);

    ArgParser arg_parser(argc, argv);
    if(!arg_parser.cmdOptionExists("-s")){
        LOG(ERROR) << "Option -s not found";
        return -1;
    }
    if(!arg_parser.cmdOptionExists("-a")){
        LOG(ERROR) << "Option -a not found";
        return -1;
    }

    const uint16_t port = 69;
    std::string host = arg_parser.getCmdOption("-s");
    std::string action = arg_parser.getCmdOption("-a");
    TFTPClient::RequestType request_type;

    if(action == "get"){
        request_type = TFTPClient::RequestType::GET;
    }
    else if (action == "put"){
        request_type = TFTPClient::RequestType::PUT;
    }
    else{
        LOG(ERROR) << "Unknow action";
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
        fname = (action == "get") ?
         TFTPClient::GetDownloadedDefaultFName() 
         : TFTPClient::GetUploadedUniqueFName();
    }
    
    LOG(INFO) << std::format("Start TFTP Client {}:{}", host.c_str(), port);

    Agent agent(host, "secret");
    agent.DoIteration();
    
    return 0;
}
