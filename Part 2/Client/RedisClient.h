#ifndef REDIS_CLIENT_H
#define REDIS_CLIENT_H

#include <string>
#include <iostream>
#include <netdb.h>
#include <sys/socket.h>
#include <unistd.h>
#include <cstring>

class RedisClient {
public:
    RedisClient(const std::string &host, int port);
    ~RedisClient();

    bool connectToServer();
    void disconnect();

private:
    std::string host;
    int port;
    int sockfd;
};

#endif // REDIS_CLIENT_H
