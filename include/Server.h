#ifndef SERVER_H
#define SERVER_H

#include <winsock2.h>
#include <ws2tcpip.h>
#include <atomic>
#include <mutex>
#include <map>
#include <string>

class Server {
private:
    SOCKET server_fd;               // server socket
    std::atomic<bool> is_running;   // control flag for server loop
    int port;

    std::mutex mtx;                 // mutex for thread safety
    std::map<SOCKET, std::string> clients; // connected clients

    // Internal helper functions
    void setup_server();
    void accept_connections();
    void handle_client(SOCKET client);
    void broadcast_message(const std::string& msg, SOCKET sender);

public:
    Server();
    void run();
    void load_configuration();
    void stop();  // gracefully stop the server
};

#endif // SERVER_H
