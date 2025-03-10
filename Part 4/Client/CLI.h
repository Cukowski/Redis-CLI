#ifndef CLI_H
#define CLI_H

#include <string>
//....
#include "RedisClient.h"
#include "CommandHandler.h"

class CLI {
public:
    CLI(const std::string &host, int port);
    void run();

private:
    RedisClient redisClient;
};

#endif // CLI_H