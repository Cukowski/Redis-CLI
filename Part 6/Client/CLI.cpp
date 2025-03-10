#include "CLI.h"
#include <vector>

// Simple helper to trim whitespace
static std::string trim(const std::string &s) {
    size_t start = s.find_first_not_of(" \t\n\r\f\v");
    if (start == std::string::npos) return "";
    size_t end = s.find_last_not_of(" \t\n\r\f\v");
    return s.substr(start, end - start + 1);
}

void printHelp() {
    std::cout << "my_redis_cli 1.0.0\n"
              << "Usage: \n"
              << "      With arguments:            ./my_redis_cli -h <host> -p <port>\n"
              << "      Default Host (127.0.0.1):  ./my_redis_cli -p <port>\n"
              << "      Default Port (6379):       ./my_redis_cli -h <host>\n"
              << "      One-shot execution:        ./my_redis_cli <command> [arguments]\n"
              << "\n"
              << "Interactive Mode (REPL):\n"
              << "      ./my_redis_cli\n"
              << "      Type Redis commands directly.\n"
              << "\n"
              << "To get help about Redis commands type:\n"
              << "      \"help\" to display this help message\n"
              << "      \"quit\" to exit\n"
              << "\n"
              << "Examples:\n"
              << "      ./my_redis_cli PING\n"
              << "      ./my_redis_cli SET mykey \"Hello World\"\n"
              << "      ./my_redis_cli GET mykey\n"
              << "\n"
              << "To set my_redis_cli preferences:\n"
              << "      \":set hints\" enable online hints\n"
              << "      \":set nohints\" disable online hints\n"
              << "Set your preferences in ~/.myredisclirc\n"
              << std::endl;
}

CLI::CLI(const std::string &host, int port) 
    : host(host), port(port), redisClient(host, port) {}

void CLI::run(const std::vector<std::string>& commandArgs) {
    if (!redisClient.connectToServer()) {
        return;
    }

    if (!commandArgs.empty()) {
        executeCommand(commandArgs);
    }

    std::cout << "Connected to Redis at " << redisClient.getSocketFD() << "\n";

    while (true) {
        std::cout << host  << ":" << port << "> ";
        std::cout.flush();
        std::string line;
        if (!std::getline(std::cin, line)) break;
        line = trim(line);
        if(line.empty()) continue;
        if (line == "quit" || line == "exit") {
            std::cout << "Goodbye.\n";
            break;
        }
        if (line == "help") {
            printHelp();
            continue;
        }

        // Split command into tokens
        std::vector<std::string> args = CommandHandler::splitArgs(line);
        if(args.empty()) continue;
        // for (const auto &arg : args) {
        //     std::cout << arg << "\n";
        // }
        std::string command = CommandHandler::buildRESPcommand(args);
        if (!redisClient.sendCommand(command)) {
            std::cerr << "(Error) Failed to send command.\n";
            break;
        }

        // Parse and print response
        std::string response = ResponseParser::parseResponse(redisClient.getSocketFD());
        std::cout << response << "\n";
    }
    redisClient.disconnect();
}

void CLI::executeCommand(const std::vector<std::string>& args) {
    if (args.empty()) return;

    std::string command = CommandHandler::buildRESPcommand(args);
    if (!redisClient.sendCommand(command)) {
        std::cerr << "(Error) Failed to send command.\n";
        return;
    }

    // Parse and print response
    std::string response = ResponseParser::parseResponse(redisClient.getSocketFD());
    std::cout << response << "\n";
}