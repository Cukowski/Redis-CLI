/*
1. Command-Line Argument Parsing
    The CLI supports optional arguments:
    -h <host> → Specify the Redis server hostname (default: 127.0.0.1).
    -p <port> → Specify the Redis server port (default: 6379).
    If no extra arguments are provided, it launches the interactive REPL mode.
2. Object-Oriented Refactoring
    Instead of one large class, we refactored the code into four key components:
    RedisClient → Handles network connection and command transmission.
    CommandHandler → Parses user input and builds RESP-formatted commands.
    ResponseParser → Parses Redis responses from the server.
    CLI → Manages the interactive REPL loop, handling user input/output.
3. Establishing a TCP Connection to Redis (RedisClient)
    Uses Berkeley sockets to open a TCP connection to the Redis server.
    Supports IPv4 and IPv6 resolution using getaddrinfo.
    Implements:
    connectToServer() → Establishes the connection.
    sendCommand() → Sends a command over the socket.
    disconnect() → Closes the socket when finished.
4. Parsing and Formatting Commands (CommandHandler)
    Splits user input into Redis command tokens, handling:
    Simple words (SET key value).
    Quoted strings (SET "hello world" 42).
    Converts commands into RESP format:
    ```
    *3\r\n
    $3\r\nSET\r\n
    $3\r\nkey\r\n
    $5\r\nvalue\r\n
    ```
5. Handling Redis Responses (ResponseParser)
    Reads server responses and parses different RESP types:
    +OK → Simple string
    -Error message → Error response
    :100 → Integer response
    $5\r\nhello → Bulk string
    *2\r\n$3\r\nfoo\r\n$3\r\nbar → Array response
6. Implementing Interactive REPL (CLI)
    Runs a loop that:
    Reads user input.
    Parses it into a valid Redis command.
    Sends the command to the server.
    Displays the parsed response.
    Supports:
    help → Displays command help.
    quit → Exits REPL.
7. Cleaning Up main.cpp
    main.cpp now only:
    Parses command-line arguments.
    Instantiates CLI and launches REPL mode.
    Single responsibility principle is followed—no networking or command logic in main.cpp.

This project now demonstrates:
    Socket Programming → Using TCP sockets in C++.
    Protocol Handling → Implementing RESP serialization/deserialization.
    OOP Principles → Following separation of concerns and single responsibility.
    CLI Development → Creating an interactive terminal-based tool.
*/

#include <iostream>

int main(int argc, char* argv[]) {
    std::string host = "127.0.0.1";
    int port = 6379;
    int i = 1;

    // Parse command-line args for -h and -p
    while (i < argc) {
        std::string arg = argv[i];
        if (arg == "-h" && i + 1 < argc) {// -h 127.0.0.1
            host = argv[++i];
        } else if (arg == "-p" && i + 1 < argc) {
            port = std::stoi(argv[++i]);
        } else {
            break;
        }
        ++i;
    }
}