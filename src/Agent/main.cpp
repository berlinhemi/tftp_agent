
#include "ArgParser.h"
#include "Packer.h"
#include "TFTPClient.h"

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
        "Usage:   {0} -h HOST -o OPERATION [-f FNAME] \n"  
        "\t-h: hostname\n"
        "\t-o: operation\n"
        "\t-f: filename (optional). Default filename for GET is 'input', for PUT is 'output'\n"
        "Examples:\n"
        "\t{0} -h 192.168.1.104 -o get \n"
        "\t{0} -h server.com -o put -f result\n"
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
    
    TFTPClient::Status status;
    Packer packer("s3cr3t_k3y");
    if (request_type == TFTPClient::RequestType::GET)
    {
        std::vector<BYTE> command;
        status = client.Get(command, fname); 
        std::vector<BYTE> unpacked =  packer.Unpack(command);
        std::ostringstream oss;
        LOG(INFO) << "Command size:" << unpacked.size();
        oss << "GetCommand result: ";
        for (auto e : unpacked){
            oss << e << " ";
        }
        LOG(INFO) << oss.str();
    }
    else if (request_type == TFTPClient::RequestType::PUT)
    {
        // Test data
        
        std::vector<BYTE> data(2000, '1');
        std::vector<BYTE> packed_data = packer.Pack(data);
        status = client.Put(packed_data, fname);
    }
  

    if (status != TFTPClient::Status::kSuccess) {
        LOG(ERROR) << client.ErrorDescription(status).c_str();
        return 1;
    }
  
  
    return 0;
}
