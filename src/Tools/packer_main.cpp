// src/tools/pack_main.cpp
//
// Утилита для подготовки задания для TFTP-агента:
//   - упаковывает (сжимает + шифрует) локальный файл
//   - опционально проверяет round-trip (распаковка == исходник)
//
// Примеры:
//   tftp-pack --key mykey123 --input task.txt --output task.packed
//   tftp-pack --key mykey123 --input task.txt --output task.packed --verify

#include "../Agent/Packer.h"

#include <cstdint>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

#include "easylogging++.h"
#include "easylogging++.cc"
INITIALIZE_EASYLOGGINGPP

namespace {

using Byte = unsigned char;

struct Options {
    std::string key;            
    std::string input;          // local file with command
    std::string output;         // output file
    bool verify = false;        
    bool help = false;
};

void PrintUsage(const char* argv0) {
    std::cout <<
        "Usage: " << argv0 << " --key <key> --input <file> --output <file> [options]\n"
        "\n"
        "Required:\n"
        "  --key <key>            Encryption key (same as agent's key)\n"
        "  --input <file>         Local file with command to pack\n"
        "  --output <file>        Write packed data to this file\n"
        "\n"
        "Options:\n"
        "  --verify               Verify round-trip after packing\n"
        "  --help                 Show this help\n";
}

Options ParseArgs(int argc, char** argv) {
    Options opt;
    for (int i = 1; i < argc; ++i) {
        std::string a = argv[i];
        auto need = [&](const char* name) -> std::string {
            if (i + 1 >= argc) {
                throw std::runtime_error(std::string("Missing value for ") + name);
            }
            return argv[++i];
        };

        if (a == "--key")         opt.key = need("--key");
        else if (a == "--input")  opt.input = need("--input");
        else if (a == "--output") opt.output = need("--output");
        else if (a == "--verify") opt.verify = true;
        else if (a == "--help" || a == "-h") { opt.help = true; return opt; }
        else throw std::runtime_error("Unknown argument: " + a);
    }
    return opt;
}

std::vector<Byte> ReadFile(const std::string& path) {
    std::ifstream in(path, std::ios::binary);
    if (!in) throw std::runtime_error("Cannot open input file: " + path);
    return std::vector<Byte>((std::istreambuf_iterator<char>(in)),
                             std::istreambuf_iterator<char>());
}

void WriteFile(const std::string& path, const std::vector<Byte>& data) {
    std::ofstream out(path, std::ios::binary);
    if (!out) throw std::runtime_error("Cannot open output file: " + path);
    out.write(reinterpret_cast<const char*>(data.data()),
              static_cast<std::streamsize>(data.size()));
    if (!out) throw std::runtime_error("Failed to write output file: " + path);
}

} // namespace


static void ConfigureLogger()
{
    el::Configurations default_conf;
    default_conf.setToDefault();
    default_conf.set(el::Level::Info, el::ConfigurationType::Enabled, "true");
    default_conf.set(el::Level::Debug, el::ConfigurationType::Enabled, "false");
    default_conf.set(el::Level::Error, el::ConfigurationType::Enabled, "true");
    default_conf.set(el::Level::Warning, el::ConfigurationType::Enabled, "true");

    el::Loggers::reconfigureLogger("default", default_conf);
    el::Loggers::reconfigureLogger("default", el::ConfigurationType::Format, "[%level] %msg");
    el::Loggers::reconfigureLogger("default", el::ConfigurationType::ToFile, "false");
}

int main(int argc, char** argv) {
    ConfigureLogger();

    try {
        Options opt = ParseArgs(argc, argv);

        if (opt.help) {
            PrintUsage(argv[0]);
            return 0;
        }
        if (opt.key.empty())    throw std::runtime_error("--key is required");
        if (opt.input.empty())  throw std::runtime_error("--input is required");
        if (opt.output.empty()) throw std::runtime_error("--output is required");

        std::vector<Byte> plain = ReadFile(opt.input);
        LOG(INFO) << "input:  " << opt.input
                  << " (" << plain.size() << " bytes)";

        Packer packer(opt.key);
        std::vector<Byte> packed = packer.Pack(plain);
        if (packed.empty() && !plain.empty()) {
            LOG(ERROR) << "Packer::Pack returned empty result";
            return 1;
        }
        LOG(INFO) << "packed: " << packed.size() << " bytes";

        if (opt.verify) {
            Packer verifier(opt.key);
            std::vector<Byte> unpacked = verifier.Unpack(packed);
            if (unpacked != plain) {
                LOG(ERROR) << "Round-trip verification failed: unpacked data differs from input";
                return 1;
            }
            LOG(INFO) << "verify: OK (round-trip matches)";
        }

        WriteFile(opt.output, packed);
        LOG(INFO) << "wrote:  " << opt.output;

        return 0;
    } catch (const std::exception& e) {
        LOG(ERROR) << "error: " << e.what();
        return 1;
    }
}