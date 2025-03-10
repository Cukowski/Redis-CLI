#ifndef RESPONSEPARSER_H
#define RESPONSEPARSER_H

#include <string>

class ResponseParser {
public:
    // Read from the given socket and return parsed response a string, return "" in failure.
    static std::string parseResponse(int sockfd);
private:
    // Redis Serialization Protocol 2
    static std::string parseSimpleString(int sockfd);
    static std::string parseSimpleError(int sockfd);
    static std::string parseInteger(int sockfd);
    static std::string parseBulkString(int sockfd);
    static std::string parseArray(int sockfd);
};

#endif // RESPONSEPARSER_H