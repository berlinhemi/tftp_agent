// src/tools/unpack_main.cpp
//
// Утилита для проверки/отладки результата TFTP-агента:
//   - распаковывает (расшифровывает + распаковывает) локальный файл
//
// Примеры:
//   tftp-unpack --key mykey123 --input result.packed --output result.txt
//   tftp-unpack --key mykey123 --input result.packed --output result.txt --verify

#include "../Agent/Packer.h"

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
    std::string key;            // обязательный
    std::string input;          // упакованный файл
    std::string output;         // куда записать распакованные данные
    bool verify = false;        // проверить round-trip (упаковать обратно == исходник)
    bool help = false;
};

void PrintUsage(const char* argv0) {
    std::cout <<
        "Usage: " << argv0 << " --key <key> --input <file> --output <file> [options]\n"
        "\n"
        "Required:\n"
        "  --key <key>            Encryption key (same as agent's key)\n"
        "  --input <file>         Packed file to unpack\n"
        "  --output <file>        Write unpacked data to this file\n"
        "\n"
        "Options:\n"
        "  --verify               Verify round-trip (pack(unpack(input)) == input)\n"
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

        // 1. Читаем упакованный файл
        std::vector<Byte> packed = ReadFile(opt.input);
        LOG(INFO) << "input:  " << opt.input
                  << " (" << packed.size() << " bytes)";

        // 2. Распаковываем (расшифровка + декомпрессия) тем же Packer'ом
        Packer packer(opt.key);
        std::vector<Byte> plain = packer.Unpack(packed);

        // Пустой результат может быть валидным (если исходник был пуст),
        // поэтому проверяем только когда входные данные непустые.
        if (plain.empty() && !packed.empty()) {
            LOG(ERROR) << "Packer::Unpack returned empty result "
                          "(wrong key or corrupted data?)";
            return 1;
        }
        LOG(INFO) << "unpacked: " << plain.size() << " bytes";

        // 3. Verify (round-trip): запаковать обратно и сравнить с исходным packed
        if (opt.verify) {
            Packer verifier(opt.key);
            std::vector<Byte> repacked = verifier.Pack(plain);
            if (repacked != packed) {
                LOG(ERROR) << "Round-trip verification failed: "
                              "pack(unpack(input)) != input";
                return 1;
            }
            LOG(INFO) << "verify: OK (round-trip matches)";
        }

        // 4. Записываем результат
        WriteFile(opt.output, plain);
        LOG(INFO) << "wrote:  " << opt.output;

        return 0;
    } catch (const std::exception& e) {
        LOG(ERROR) << "error: " << e.what();
        return 1;
    }
}