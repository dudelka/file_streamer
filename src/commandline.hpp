#pragma once

#include <cstdint>
#include <string>

#ifdef CLIENT_MODE
#include <vector>
#endif

void Usage(const std::string& program_name);
void PrintUsage(const std::string& program_name);
void LogErrorAndUsage(const std::string& error, const std::string& program_name);

class CommandlineParser {
public:
    CommandlineParser(const int argc, const char* argv[]);

#ifdef CLIENT_MODE
    const std::vector<std::string>* GetFiles() const;
#endif
    std::string GetSendAddress() const;
    std::string GetReceiveAddress() const;
    uint32_t GetTimeout() const;

private:
#ifdef CLIENT_MODE
    std::vector<std::string> files_;
#endif
    std::string send_address_;
    std::string receive_address_;
    uint32_t timeout_ {0};

    void ParseArgs(const int argc, const char* argv[]);
};
