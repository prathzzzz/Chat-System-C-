// client.cpp
#include <iostream>
#include <string>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <conio.h>
#include <thread>

#pragma comment(lib, "ws2_32.lib")

using namespace std;

class ChatClient {
private:
    SOCKET sock;
    bool running;
    thread receive_thread;

    void receiveMessages() {
        char buffer[1024];
        while (running) {
            memset(buffer, 0, sizeof(buffer));
            int bytesRead = recv(sock, buffer, sizeof(buffer), 0);
            
            if (bytesRead <= 0) {
                cout << "Disconnected from server." << endl;
                running = false;
                break;
            }
            
            cout << "\r" << buffer << endl;  // \r to clear any partial input line
            cout << "> " << flush;  // Restore input prompt
        }
    }

public:
    ChatClient(const string& serverIP, int port, const string& username) {
        running = true;
        // Initialize Winsock
        WSADATA wsaData;
        if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
            throw runtime_error("WSAStartup failed");
        }

        // Create socket
        sock = socket(AF_INET, SOCK_STREAM, 0);
        if (sock == INVALID_SOCKET) {
            WSACleanup();
            throw runtime_error("Failed to create socket");
        }

        // Connect to server
        sockaddr_in serverAddr{};
        serverAddr.sin_family = AF_INET;
        serverAddr.sin_port = htons(port);
        inet_pton(AF_INET, serverIP.c_str(), &serverAddr.sin_addr);

        if (connect(sock, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
            closesocket(sock);
            WSACleanup();
            throw runtime_error("Failed to connect to server");
        }

        // Send username
        send(sock, username.c_str(), username.length(), 0);
    }

    void run() {
        // Start receive thread
        receive_thread = thread(&ChatClient::receiveMessages, this);
        
        string input;
        cout << "> " << flush;
        
        while (running) {
            char c = _getch();
            if (c == '\r' || c == '\n') {  // Enter key pressed
                if (!input.empty()) {
                    cout << endl;
                    if (input == "quit") {
                        running = false;
                        break;
                    }
                    send(sock, input.c_str(), input.length(), 0);
                    input.clear();
                    cout << "> " << flush;
                }
            } else if (c == '\b') {  // Backspace
                if (!input.empty()) {
                    input.pop_back();
                    cout << "\b \b";  // Erase character from console
                }
            } else {
                input += c;
                cout << c;  // Echo character to console
            }
        }
        
        // Wait for receive thread to finish
        if (receive_thread.joinable()) {
            receive_thread.join();
        }
    }

    ~ChatClient() {
        running = false;
        if (receive_thread.joinable()) {
            receive_thread.join();
        }
        closesocket(sock);
        WSACleanup();
    }
};

int main() {
    try {
        string username;
        cout << "Enter your username: ";
        getline(cin, username);

        ChatClient client("127.0.0.1", 8080, username);
        cout << "Connected to server. Type 'quit' to exit." << endl;
        cout << "To send a private message: @username:message" << endl;
        cout << "To broadcast: message" << endl;

        client.run();
    } catch (const exception& e) {
        cerr << "Error: " << e.what() << endl;
        return 1;
    }
    return 0;
}