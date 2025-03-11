# Welcome to My Cpp Redis Client
***

## Task
This project is a **Redis Command Line Interface (CLI)** written in **C++**.  
It allows users to **connect to a Redis server** via **TCP sockets**, send **Redis commands**,  
and **receive and parse responses** using the **RESP protocol**.

This implementation supports:
- **Interactive REPL Mode** (like the official Redis CLI)
- **One-shot execution Mode** (run a single Redis command from the terminal)
- **Networking** using Berkeley sockets (IPv4 & IPv6 support)
- **RESP Protocol Parsing** for handling Redis responses (RESP2).

---

## Description
This Redis CLI was implemented using **C++** with a focus on:
**Low-level TCP connection to Redis**  
**Supports Redis commands (SET, GET, PING, etc.)**  
**Interactive REPL shell**  
**One-shot execution mode**  
**Properly formatted RESP protocol parsing**  
**Cross-platform compatibility (Linux & macOS)**  
**Object-oriented design with separate classes**  

---

## Project Structure
This project follows a **modular OOP architecture**:

| Component        | Responsibility |
|------------------|----------------|
| `RedisClient`    | Handles TCP socket connections and sending commands to Redis. |
| `CommandHandler` | Parses user input and formats Redis commands into RESP format. |
| `ResponseParser` | Parses responses from Redis (Simple Strings, Errors, Bulk Strings, Arrays, etc.). |
| `CLI`            | Manages the **REPL loop**, user input, and command execution. |
| `main.cpp`       | Parses command-line arguments and launches the CLI. |

---

## Installation
You can use terminal to build, make and use.

### **Requirements**
- **G++ (C++ Compiler)**
- **Make**

---

### **Build Instructions**
```sh
git clone https://git.us.qwasar.io/my_cpp_redis_client_180048_hkz9ke/my_cpp_redis_client
cd my_cpp_redis_client
make
```
This will generate the executable **`bin/my_redis_cli`**.

---

## Usage
After  generating the executable **`bin/my_redis_cli`**, you can use it.

### Start Interactive Mode (REPL)
```
./bin/my_redis_cli
```
Example:
```
127.0.0.1:6379> set mykey hello
OK
127.0.0.1:6379> get mykey
"hello"
127.0.0.1:6379> quit
Goodbye.
```

### Execute a Single Command
```
./bin/my_redis_cli set mykey "hello"
OK
./bin/my_redis_cli get mykey 
"hello"
```

### Connect to a Custom Redis Server
```
./bin/my_redis_cli -h 192.168.1.10 -p 6380
```

### Show Help
```
./bin/my_redis_cli
127.0.0.1:6379> help
```

### The Core Team
Selcuk Ata Aksoy
