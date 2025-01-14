// server.cpp
#include <iostream>
#include <map>
#include <string>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <thread>
#include <mutex>
#include <vector>

#pragma comment(lib, "ws2_32.lib")

using namespace std;

class ChatServer {
private:
    SOCKET serverSocket;
    map<string, SOCKET> clients;  // username -> socket
    mutex clients_mutex;  // Protect access to clients map
    vector<thread> client_threads;
    
    void broadcast(const string& sender, const string& message) {
        string fullMessage = sender + ": " + message;
        for (const auto& client : clients) {
            if (client.first != sender) {
                send(client.second, fullMessage.c_str(), fullMessage.length(), 0);
            }
        }
    }
    
    void sendPrivateMessage(const string& sender, const string& recipient, const string& message) {
        auto it = clients.find(recipient);
        if (it != clients.end()) {
            string fullMessage = "Private from " + sender + ": " + message;
            send(it->second, fullMessage.c_str(), fullMessage.length(), 0);
        }
    }

    void handleClient(SOCKET clientSocket) {
        // Get username
        char username[1024] = {0};
        recv(clientSocket, username, sizeof(username), 0);
        string user(username);
        
        {
            lock_guard<mutex> lock(clients_mutex);
            clients[user] = clientSocket;
        }
        
        cout << "New client connected: " << user << endl;
        
        while (true) {
            char buffer[1024] = {0};
            int bytesRead = recv(clientSocket, buffer, sizeof(buffer), 0);
            
            if (bytesRead <= 0) {
                // Client disconnected
                {
                    lock_guard<mutex> lock(clients_mutex);
                    clients.erase(user);
                }
                cout << "Client disconnected: " << user << endl;
                closesocket(clientSocket);
                break;
            }
            
            string message(buffer);
            if (message[0] == '@') {
                // Private message
                size_t colonPos = message.find(':');
                if (colonPos != string::npos) {
                    string recipient = message.substr(1, colonPos - 1);
                    string content = message.substr(colonPos + 1);
                    sendPrivateMessage(user, recipient, content);
                }
            } else {
                // Broadcast message
                broadcast(user, message);
            }
        }
    }

public:
    ChatServer(int port) {
        // Initialize Winsock
        WSADATA wsaData;
        if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
            throw runtime_error("WSAStartup failed");
        }

        // Create socket
        serverSocket = socket(AF_INET, SOCK_STREAM, 0);
        if (serverSocket == INVALID_SOCKET) {
            WSACleanup();
            throw runtime_error("Failed to create server socket");
        }
        
        // Set socket options to reuse address
        char opt = 1;
        setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
        
        // Set up server address
        sockaddr_in serverAddr{};
        serverAddr.sin_family = AF_INET;
        serverAddr.sin_addr.s_addr = INADDR_ANY;
        serverAddr.sin_port = htons(port);
        
        // Bind
        if (bind(serverSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
            closesocket(serverSocket);
            WSACleanup();
            throw runtime_error("Failed to bind server socket");
        }
        
        // Listen
        if (listen(serverSocket, 5) == SOCKET_ERROR) {
            closesocket(serverSocket);
            WSACleanup();
            throw runtime_error("Failed to listen on server socket");
        }
    }
    
    void start() {
        cout << "Server started. Waiting for connections..." << endl;
        
        while (true) {
            sockaddr_in clientAddr{};
            int clientLen = sizeof(clientAddr);
            SOCKET clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddr, &clientLen);
            
            if (clientSocket == INVALID_SOCKET) {
                cerr << "Failed to accept client connection" << endl;
                continue;
            }
            
            // Create new thread for client
            client_threads.emplace_back(&ChatServer::handleClient, this, clientSocket);
        }
    }
    
    ~ChatServer() {
        // Join all client threads
        for (auto& thread : client_threads) {
            if (thread.joinable()) {
                thread.join();
            }
        }
        
        {
            lock_guard<mutex> lock(clients_mutex);
            for (const auto& client : clients) {
                closesocket(client.second);
            }
        }
        closesocket(serverSocket);
        WSACleanup();
    }
};

int main() {
    try {
        ChatServer server(8080);
        server.start();
    } catch (const exception& e) {
        cerr << "Server error: " << e.what() << endl;
        return 1;
    }
    return 0;
}