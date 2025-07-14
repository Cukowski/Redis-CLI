#include "CLI.h"
#include <vector>
#include <poll.h>
#include <readline/readline.h>
#include <readline/history.h>

static bool lineReady = false;
static std::string latestInput = "";

// readline callback when a full line is ready
static void handleLine(char* line) {
    if (!line) {
        lineReady = true;
        latestInput = "exit";  // treat Ctrl+D as exit
        return;
    }
    latestInput = std::string(line);
    // Add to history for Up/Down navigation
    add_history(latestInput.c_str());
    free(line);
    lineReady = true;
}

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
    bool readlineActive = false;

    if (!redisClient.connectToServer()) {
        return;
    }

    if (!commandArgs.empty()) {
        executeCommand(commandArgs);
    }

    std::cout << "Connected to Redis at " << redisClient.getSocketFD() << "\n";

    int sockfd = redisClient.getSocketFD();
    
    // polling terminal input and redis
    struct pollfd fds[2];
    fds[0].fd = STDIN_FILENO;  // Terminal input
    fds[0].events = POLLIN;
    fds[1].fd = sockfd;        // Redis socket
    fds[1].events = POLLIN;

    while (true) {
        // std::cout << host  << ":" << port << "> ";
        // std::cout.flush();
        
        rl_callback_handler_install((host + ":" + std::to_string(port) + "> ").c_str(), handleLine);
        readlineActive = true;

        int ret = poll(fds, 2, -1);  // Block indefinitely
        if (ret < 0) {
            perror("(Error) Poll failed");
            if (readlineActive) {
                rl_callback_handler_remove();
                readlineActive = false;
            }
            break;
        }

        // Handle disconnection or socket error
        if (fds[1].revents & (POLLHUP | POLLERR)) {
            std::cout << "\nRedis connection lost. Exiting...\n";
            if (readlineActive) {
                rl_callback_handler_remove();
                readlineActive = false;
            }
            break;
        }

        // Socket is readable — could be a response or a closed connection (EOF)
        if (fds[1].revents & POLLIN) {
            char buffer[1];
            ssize_t bytes = recv(sockfd, buffer, sizeof(buffer), MSG_PEEK);
            if (bytes == 0) {
                std::cout << "\nRedis server closed the connection. Exiting...\n" << std::endl;
                if (readlineActive) {
                    rl_callback_handler_remove();
                    readlineActive = false;
                }
                break;
            }
        }

        if (fds[0].revents & POLLIN) {
            rl_callback_read_char();
        }

        if (lineReady) {
            std::string line = trim(latestInput);
            // if (!std::getline(std::cin, line)) break;
            lineReady = false;

            if (line == "quit" || line == "exit") {
                std::cout << "Goodbye.\n";
                if (readlineActive) {
                    rl_callback_handler_remove();
                    readlineActive = false;
                }
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

            // subscription
            std::string firstCmd = args[0];
            std::transform(firstCmd.begin(), firstCmd.end(), firstCmd.begin(), ::toupper);
    
            // std::cout<<"first command check : "<<firstCmd<<std::endl;

            if (firstCmd == "SUBSCRIBE") {
                if (readlineActive) {
                    rl_callback_handler_remove();
                    readlineActive = false;
                }
                handleSubscription(args);
                rl_callback_handler_install((host + ":" + std::to_string(port) + "> ").c_str(), handleLine);
                readlineActive = true;
                continue;  // skip rest of loop
            }
    
            std::string command = CommandHandler::buildRESPcommand(args);
            if (!redisClient.sendCommand(command)) {
                std::cerr << "(Error) Failed to send command.\n";
                std::cout.flush();
                if (readlineActive) {
                    rl_callback_handler_remove();
                    readlineActive = false;
                }
                break;
            }
            // Parse and print response
            try {
                std::string response = ResponseParser::parseResponse(redisClient.getSocketFD());
                std::cout << response << "\n";
            } catch (const std::exception &e) {
                std::cerr << "(Error) Failed to parse response: " << e.what() << "\n";
                std::cerr << "Redis server might have disconnected.\n";
                if (readlineActive) {
                    rl_callback_handler_remove();
                    readlineActive = false;
                }
                break;
            } catch (...) {
                std::cerr << "(Error) Unknown error during response parsing.\n";
                if (readlineActive) {
                    rl_callback_handler_remove();
                    readlineActive = false;
                }
                break;
            }
        }
    }
    rl_callback_handler_remove();
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
    try {
        std::string response = ResponseParser::parseResponse(redisClient.getSocketFD());
        std::cout << response << "\n";
    } catch (const std::exception &e) {
        std::cerr << "(Error) Failed to parse response: " << e.what() << "\n";
        std::cerr << "Redis server might have disconnected.\n";
    } catch (...) {
        std::cerr << "(Error) Unknown error during response parsing.\n";
    }
}

// handles subscription
void CLI::handleSubscription(const std::vector<std::string>& args) {
    std::string command = CommandHandler::buildRESPcommand(args);
    if (!redisClient.sendCommand(command)) {
        std::cerr << "(Error) Failed to send SUBSCRIBE command.\n";
        return;
    }

    std::cout << "(Subscribed) Type 'exit'/'quit' to quit subscription mode.\n";

    int sockfd = redisClient.getSocketFD();

    struct pollfd fds[2];
    fds[0].fd = STDIN_FILENO;  // user input
    fds[0].events = POLLIN;
    fds[1].fd = sockfd;        // Redis socket
    fds[1].events = POLLIN;

    // install readline handler
    rl_callback_handler_install("", handleLine);
    
    bool inSubscription = true;
    while (inSubscription) {
        rl_callback_handler_install("", handleLine);
        int ret = poll(fds, 2, 100);  // timeout in 100ms

        if (ret < 0) {
            perror("(Error) Poll failed");
            break;
        }

        // Redis connection closed?
        if (fds[1].revents & (POLLHUP | POLLERR)) {
            std::cout << "\nRedis connection lost. Exiting...\n";
            rl_callback_handler_remove();
            exit(1);
        }

        if (fds[1].revents & POLLIN) {
            // Check if Redis server closed socket
            char buffer[1];
            ssize_t bytes = recv(sockfd, buffer, sizeof(buffer), MSG_PEEK);
            if (bytes == 0) {
                std::cout << "\nRedis server closed the connection. Exiting...\n";
                rl_callback_handler_remove();
                exit(1);
            }

            try {
                std::string message = ResponseParser::parseResponse(sockfd);
                std::cout << message << std::endl;
            } catch (const std::exception &e) {
                std::cerr << "(Error) Failed to parse pub/sub message: " << e.what() << "\n";
                rl_callback_handler_remove();
                exit(1);
            }
        }

        // Terminal input
        if (fds[0].revents & POLLIN) {
            rl_callback_read_char();
        }

        // If a full line is available
        if (lineReady) {
            std::string input = trim(latestInput);
            lineReady = false;

            if (input == "exit" || input == "quit") {
                std::vector<std::string> unsubCmd = {"UNSUBSCRIBE"};
                std::string unsubCommand = CommandHandler::buildRESPcommand(unsubCmd);
                redisClient.sendCommand(unsubCommand);
                inSubscription = false;
            } else {
                std::cout << "(Info) Type 'exit'/'quit' to leave subscription mode.\n";
            }
        }
    }

    rl_callback_handler_remove();
    std::cout << "(Exited subscription mode)\n";
}