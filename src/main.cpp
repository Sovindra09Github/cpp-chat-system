#include "Server.h"
#include <iostream>
#include <winsock2.h>

int main() {
    WSADATA wsaData;

    // 1️⃣ Initialize Winsock
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "WSAStartup failed!" << std::endl;
        return 1;
    }

    try {
        Server chat_server;
        chat_server.run();
    }
    catch (const std::exception& e) {
        std::cerr << "An unexpected error occurred: " << e.what() << std::endl;
        WSACleanup();
        return 1;
    }
    catch (...) {
        std::cerr << "An unknown error occurred." << std::endl;
        WSACleanup();
        return 1;
    }

    // 2️⃣ Cleanup Winsock
    WSACleanup();
    return 0;
}
