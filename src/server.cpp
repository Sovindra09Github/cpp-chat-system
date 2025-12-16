#include "Server.h"
#include <iostream>
#include <fstream>
#include <thread>
#include <cstring>
#include <algorithm>

using namespace std;

// Helper function: case-insensitive string comparison
bool iequals(const std::string& a, const std::string& b) {
    if (a.size() != b.size()) return false;
    for (size_t i = 0; i < a.size(); ++i) {
        if (tolower(a[i]) != tolower(b[i])) return false;
    }
    return true;
}

// Constructor
Server::Server()
    : server_fd(INVALID_SOCKET),
      is_running(true),
      port(8080) // default port
{
}

// Run server
void Server::run() {
    load_configuration();
    setup_server();

    // Start accepting clients in a separate thread
    thread accept_thread(&Server::accept_connections, this);

    // Server console input for shutdown
    string cmd;
    while (true) {
        getline(cin, cmd);
        if (cmd == "/shutdown") {
            stop();
            break;
        }
    }

    accept_thread.join(); // wait for accept thread to finish
}

// Load port from configuration
void Server::load_configuration() {
    ifstream file("server.conf");
    if (file) {
        string line;
        while (getline(file, line)) {
            if (line.find("PORT=") == 0) {
                port = stoi(line.substr(5));
            }
        }
    }
}

// Setup server socket
void Server::setup_server() {
    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port);

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == INVALID_SOCKET) {
        cerr << "Socket creation failed\n";
        exit(1);
    }

    if (bind(server_fd, (sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR) {
        cerr << "Bind failed\n";
        exit(1);
    }

    if (listen(server_fd, 10) == SOCKET_ERROR) {
        cerr << "Listen failed\n";
        exit(1);
    }

    cout << "Server running on port " << port << endl;
    cout << "Type /shutdown to stop the server gracefully." << endl;
}

// Accept incoming clients
void Server::accept_connections() {
    while (is_running) {
        SOCKET client = accept(server_fd, nullptr, nullptr);
        if (client == INVALID_SOCKET) {
            if (!is_running) break; // exit loop if shutting down
            continue;
        }

        {
            lock_guard<mutex> lock(mtx);
            clients[client] = "User" + to_string(client);
        }

        cout << "Client connected: " << client << endl;

        thread(&Server::handle_client, this, client).detach();
    }
}

// Handle a single client
void Server::handle_client(SOCKET client) {
    char buffer[1024];
    int bytes;

    string name;
    {
        lock_guard<mutex> lock(mtx);
        name = clients[client];
    }

    while ((bytes = recv(client, buffer, sizeof(buffer), 0)) > 0 && is_running) {
        string msg(buffer, bytes);

        // /nick command
        if (msg.rfind("/nick ", 0) == 0) {
            string new_name = msg.substr(6);
            if (!new_name.empty()) {
                lock_guard<mutex> lock(mtx);
                clients[client] = new_name;
                string notify = "Server: You are now known as " + new_name;
                send(client, notify.c_str(), notify.size(), 0);
                name = new_name; // update local copy
            }
            continue;
        }

        // /msg command (private, case-insensitive)
        if (msg.rfind("/msg ", 0) == 0) {
            size_t space1 = msg.find(' ', 5);
            if (space1 != string::npos) {
                string target_name = msg.substr(5, space1 - 5);
                string private_msg = msg.substr(space1 + 1);

                bool found = false;
                lock_guard<mutex> lock(mtx);
                for (auto& [sock, nick] : clients) {
                    if (iequals(nick, target_name)) {
                        string full = "(Private) " + name + ": " + private_msg;
                        send(sock, full.c_str(), full.size(), 0);
                        found = true;
                        break;
                    }
                }

                if (!found) {
                    string error_msg = "Server: User '" + target_name + "' not found.";
                    send(client, error_msg.c_str(), error_msg.size(), 0);
                }
            } else {
                string error_msg = "Server: Usage: /msg <username> <message>";
                send(client, error_msg.c_str(), error_msg.size(), 0);
            }
            continue;
        }

        // /list command
        if (msg == "/list") {
            lock_guard<mutex> lock(mtx);
            string user_list = "Online users:\n";
            for (auto& [_, nick] : clients) {
                user_list += "- " + nick + "\n";
            }
            send(client, user_list.c_str(), user_list.size(), 0);
            continue;
        }

        // /exit command
        if (msg == "/exit") {
            break;
        }

        // Broadcast normal messages
        string full = name + ": " + msg;
        broadcast_message(full, client);
    }

    {
        lock_guard<mutex> lock(mtx);
        clients.erase(client);
    }

    closesocket(client);
}

// Broadcast message to all clients except sender
void Server::broadcast_message(const string& msg, SOCKET sender) {
    lock_guard<mutex> lock(mtx);
    for (auto& [sock, _] : clients) {
        if (sock != sender) {
            send(sock, msg.c_str(), msg.size(), 0);
        }
    }
}

// Gracefully stop the server
void Server::stop() {
    is_running = false;

    // Close all client sockets
    lock_guard<mutex> lock(mtx);
    for (auto& [sock, _] : clients) {
        send(sock, "Server is shutting down.", 23, 0);
        closesocket(sock);
    }
    clients.clear();

    // Close server socket
    closesocket(server_fd);

    cout << "Server stopped gracefully." << endl;
}
