#ifndef COMMAND_HANDLER_H
#define COMMAND_HANDLER_H

#include <vector>
#include <string>

class CommandHandler {
public:
    // Split command into tokens
    static std::vector<std::string> splitArgs(const std::string &input);

    // Build a RESP command from the vector arguments
    static std::string buildRESPcommand(const std::vector<std::string> &args);
};


#endif // COMMAND_HANDLER_H