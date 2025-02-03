#include <WinSock2.h>
#include <WS2tcpip.h>
#include <iostream>
#include <string>
#include "../Global.h"
#pragma comment(lib, "Ws2_32.lib")

#define SERVER_IP "127.0.0.1" // 서버 IP
#define SERVER_PORT 9000      // 서버 포트
#define BUFFER_SIZE 512       // 버퍼 크기

void start()
{
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        std::cerr << "WSAStartup failed. Error: " << WSAGetLastError() << std::endl;
        exit(1);
    }
}

void stop()
{
    WSACleanup();
}

int main()
{
    start();

    SOCKET clientSock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (clientSock == INVALID_SOCKET) {
        std::cerr << "Socket creation failed. Error: " << WSAGetLastError() << std::endl;
        stop();
        return 1;
    }

    SOCKADDR_IN serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(SERVER_PORT);
    inet_pton(AF_INET, SERVER_IP, &serverAddr.sin_addr);

    if (connect(clientSock, reinterpret_cast<sockaddr*>(&serverAddr), sizeof(serverAddr)) == SOCKET_ERROR) {
        std::cerr << "Connection failed. Error: " << WSAGetLastError() << std::endl;
        closesocket(clientSock);
        stop();
        return 1;
    }

    std::cout << "Connected to server at " << SERVER_IP << ":" << SERVER_PORT << std::endl;

    // 메시지 보내기
    std::string message = "Hello, IOCP server!";
    int bytesSent = send(clientSock, message.c_str(), message.length(), 0);
    if (bytesSent == SOCKET_ERROR) {
        std::cerr << "Send failed. Error: " << WSAGetLastError() << std::endl;
        closesocket(clientSock);
        stop();
        return 1;
    }
    std::cout << "Sent message: " << message << std::endl;

    // 서버로부터 응답 받기
    char buffer[BUFFER_SIZE] = { 0 };
    int bytesReceived = recv(clientSock, buffer, BUFFER_SIZE - 1, 0);
    if (bytesReceived > 0) {
        buffer[bytesReceived] = '\0';
        std::cout << "Received message from server: " << buffer << std::endl;
    }
    else if (bytesReceived == 0) {
        std::cout << "Connection closed by server." << std::endl;
    }
    else {
        std::cerr << "Receive failed. Error: " << WSAGetLastError() << std::endl;
    }

    // 소켓 닫기
    closesocket(clientSock);
    stop();
    return 0;
}