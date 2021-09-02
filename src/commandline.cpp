#include "commandline.hpp"

#include <algorithm>
#include <iterator>
#include <iostream>
#include <exception>
#include <utility>
#include <list>

#include <stdlib.h>

void Usage(const std::string& program_name) {
    std::cerr << program_name << " [options]" << std::endl;
    std::cerr << "Options: " << std::endl;
#ifdef CLIENT_MODE
    std::cerr << "      --file <filename> - file to be sent." << std::endl;
    std::cerr << "      --send_address <ip:port> - address to send files." << std::endl;
    std::cerr << "      --receive_address <ip:port> - address to receive packets from server." 
        << std::endl;
    std::cerr << "      --timeout <uint32> - timeout in microseconds to resend lost packets. Default is 15 microseconds." 
        << std::endl;
#elif SERVER_MODE
    std::cerr << "      --send_address <ip:port> - address to send ack packets." << std::endl;
    std::cerr << "      --receive_address <ip:port> - address to receive packets from client." 
        << std::endl;
    std::cerr << "      --timeout <uint32> - timeout in seconds. If no input on receiver was detected it will be shutdowned. Default is 30 seconds." 
        << std::endl;
#endif
    std::cerr << "      --help - print this message." << std::endl;
#ifdef CLIENT_MODE
    std::cerr << "Note that arguments should contain at least one --file option." << std::endl;
#endif
    std::cerr << "Options --send_address and --receive_address are required." << std::endl;
}

void PrintUsage(const std::string& program_name) {
    Usage(program_name);
    throw std::invalid_argument("Wrong usage!");
}

void LogErrorAndUsage(const std::string& error, const std::string& program_name) {
    std::cerr << error << std::endl;
    PrintUsage(program_name);
}

CommandlineParser::CommandlineParser(const int argc, const char* argv[]) {
    ParseArgs(argc, argv);

#ifdef CLIENT_MODE
    if (files_.empty()) {
        LogErrorAndUsage("There is no files to be sent.",
            argv[0]);
    }
#endif
    if (send_address_.empty()) {
        LogErrorAndUsage("No send address specified in commandline arguments.", argv[0]);
    }
    if (receive_address_.empty()) {
        LogErrorAndUsage("No receive address specified in commandline arguments.", argv[0]);
    }

    PrintParsedConfig();
}

#ifdef CLIENT_MODE
const std::vector<std::string>* CommandlineParser::GetFiles() const {
    return &files_;
}
#endif

std::string CommandlineParser::GetSendAddress() const {
    return send_address_;
}

std::string CommandlineParser::GetReceiveAddress() const {
    return receive_address_;
}

uint32_t CommandlineParser::GetTimeout() const {
    return timeout_;
}

#define BREAK_IF_END(container, it, opt_name)               \
    if (it == container.end()) {                            \
        std::string error_msg = "Option " + opt_name        \
            + " was specified but no value was provided.";    \
        throw std::invalid_argument(error_msg);             \
    }

void CommandlineParser::ParseArgs(const int argc, const char* argv[]) {
    std::list<std::string> args(argv, argv + argc);
    args.pop_front(); // because first argument is program name

    auto help_it = std::find(args.begin(), args.end(), "--help");
    if (help_it != args.end()) {
        Usage(argv[0]);
        exit(0);
    }

    for (auto it = args.begin(); it != args.end(); ++it) {
        if (*it == "--send_address") {
            ++it;
            BREAK_IF_END(args, it, *std::prev(it))
            send_address_ = std::move(*it);
        } else if (*it == "--receive_address") {
            ++it;
            BREAK_IF_END(args, it, *std::prev(it))
            receive_address_ = std::move(*it);
        } 
#ifdef CLIENT_MODE
        else if (*it == "--file") {
            ++it;
            BREAK_IF_END(args, it, *std::prev(it))
            files_.emplace_back(std::move(*it));
        } 
#endif
        else if (*it == "--timeout") {
            ++it;
            BREAK_IF_END(args, it, *std::prev(it))
            timeout_ = std::stoi(*it);
        } else {
            std::cerr << "Unknown argument: " << *it << "." << std::endl;
        }
    }

    if (timeout_ == 0) {
        std::cerr << "Timeout can't be 0. Set to default." << std::endl;
#ifdef CLIENT_MODE
        timeout_ = 15;
#elif SERVER_MODE
        timeout_ = 30;
#endif
    }
}

void CommandlineParser::PrintParsedConfig() const {
    std::cout << "Parsed config:" << std::endl;
#ifdef CLIENT_MODE
    std::cout << "\tFiles to send:" << std::endl;
    for (const auto& filename : files_) {
        std::cout << "\t\t" << filename << std::endl;
    }
#endif
    std::cout << "\tSend address:" << std::endl;
    std::cout << "\t\t" << send_address_ << std::endl;

    std::cout << "\tReceive address:" << std::endl;
    std::cout << "\t\t" << receive_address_ << std::endl;
    
#ifdef CLIENT_MODE
    std::string timeout_val("microseconds");
#elif SERVER_MODE
    std::string timeout_val("seconds");
#endif
    std::cout << "\tTimeout:" << std::endl;
    std::cout << "\t\t" << timeout_ << " " << timeout_val << std::endl;
}
