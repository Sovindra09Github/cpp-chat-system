#include <iostream>
#include <thread>
#include <string>
#include <atomic>
#include <winsock2.h>
#include <ws2tcpip.h>

using namespace std;

class Client {
private:
    SOCKET sock;
    int port;
    string server_ip;
    atomic<bool> running;

    void receive_messages() {
        char buffer[1024];
        int bytes;

        while (running && (bytes = recv(sock, buffer, sizeof(buffer), 0)) > 0) {
            string msg(buffer, bytes);

            // Detect server shutdown
            if (msg == "Server is shutting down.") {
                cout << "[SERVER] " << msg << endl;
                running = false;
                break;
            }

            // Highlight private messages
            if (msg.rfind("(Private)", 0) == 0) {
                cout << "[PRIVATE] " << msg.substr(9) << endl;
            }
            // Highlight server messages
            else if (msg.rfind("Server:", 0) == 0) {
                cout << "[SERVER] " << msg.substr(7) << endl;
            }
            else {
                cout << msg << endl;
            }
        }

        running = false;
    }

public:
    Client(const string& ip = "127.0.0.1", int p = 8080)
        : sock(INVALID_SOCKET), port(p), server_ip(ip), running(true) {}

    void run() {
        WSADATA wsaData;
        if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
            cerr << "WSAStartup failed\n";
            return;
        }

        sock = socket(AF_INET, SOCK_STREAM, 0);
        if (sock == INVALID_SOCKET) {
            cerr << "Socket creation failed\n";
            WSACleanup();
            return;
        }

        sockaddr_in server_addr{};
        server_addr.sin_family = AF_INET;
        server_addr.sin_port = htons(port);
        inet_pton(AF_INET, server_ip.c_str(), &server_addr.sin_addr);

        if (connect(sock, (sockaddr*)&server_addr, sizeof(server_addr)) == SOCKET_ERROR) {
            cerr << "Connect failed\n";
            closesocket(sock);
            WSACleanup();
            return;
        }

        cout << "Connected to server " << server_ip << ":" << port << endl;
        cout << "Type messages to send. Commands: /nick, /msg, /list, /exit" << endl;

        thread recv_thread(&Client::receive_messages, this);
        recv_thread.detach();

        string msg;
        while (running && getline(cin, msg)) {
            if (msg == "/exit") {
                send(sock, msg.c_str(), msg.size(), 0);
                break;
            }
            send(sock, msg.c_str(), msg.size(), 0);
        }

        running = false;
        closesocket(sock);
        WSACleanup();
        cout << "Disconnected from server." << endl;
    }
};

int main() {
    Client client("127.0.0.1", 8080);
    client.run();
    return 0;
}
