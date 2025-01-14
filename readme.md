# Chat Application

## Overview
This project is a simple TCP-based chat application implemented in C++. It consists of a server and client, enabling multiple users to communicate with each other through broadcast and private messages.

---

## Features
- **Broadcast Messages**: Send messages to all connected clients.
- **Private Messaging**: Send a message to a specific user using the `@username:message` format.
- **Multi-Client Support**: Handle multiple clients simultaneously using multi-threading.
- **Graceful Disconnection**: Detect and handle client disconnections properly.

---

## Prerequisites
### Development Environment:
- **Operating System**: Windows
- **Compiler**: MSVC (Microsoft Visual C++) or any other compatible C++ compiler
- **Build Tools**: Visual Studio or any IDE that supports Windows Socket Programming
- **Dependencies**: WinSock2 library (already included in most Windows development environments)

---

## Setup Instructions
### Server:
1. Open the `server.cpp` file in your preferred IDE.
2. Compile and run the program.
3. The server will start listening for connections on port `8080`.

### Client:
1. Open the `client.cpp` file in your preferred IDE.
2. Compile and run the program.
3. Enter your username when prompted.
4. The client will connect to the server on `127.0.0.1` (localhost) and port `8080`.

---

## Usage
### Starting the Server:
1. Run the server executable.
2. Wait for clients to connect. The server will display a message for each connected client.

### Starting the Client:
1. Run the client executable.
2. Provide a username when prompted.
3. Use the following message formats:
   - **Broadcast**: Type your message and press `Enter`.
   - **Private**: Use the format `@username:message` to send a private message.
4. Type `quit` to disconnect.

---

