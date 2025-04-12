#include "TCPClient.h"

TCPClient::TCPClient(const std::string& serverIP, int serverPort)
    : serverIP(serverIP), serverPort(serverPort), clientSocket(INVALID_SOCKET) {
    WSADATA wsaData;
    int wsaStartupResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (wsaStartupResult != 0) {
        throw std::runtime_error("WSAStartup failed, error code: " + std::to_string(wsaStartupResult));
    }
}

TCPClient::~TCPClient() {
    disconnect();
    WSACleanup();
}

void TCPClient::connectToServer() {
    clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket == INVALID_SOCKET) {
        int errorCode = WSAGetLastError();
        throw std::runtime_error("Socket creation failed, error code: " + std::to_string(errorCode));
    }

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(serverPort);
    if (inet_pton(AF_INET, serverIP.c_str(), &serverAddr.sin_addr) <= 0) {
        closesocket(clientSocket);
        throw std::runtime_error("Invalid server IP address");
    }

    int result = connect(clientSocket, reinterpret_cast<sockaddr*>(&serverAddr), sizeof(serverAddr));
    if (result == SOCKET_ERROR) {
        int errorCode = WSAGetLastError();
        closesocket(clientSocket);
        throw std::runtime_error("Connection to server failed, error code: " + std::to_string(errorCode));
    }

    std::cout << "Connected to server" << std::endl;
}

void TCPClient::sendData(const std::string& data) {
    std::cout << "Sending command: " << data << std::endl;
    int result = send(clientSocket, data.c_str(), static_cast<int>(data.length()), 0);
    if (result == SOCKET_ERROR) {
        int errorCode = WSAGetLastError();
        throw std::runtime_error("Send data failed, error code: " + std::to_string(errorCode));
    }
}

std::string TCPClient::receiveData() {
    char buffer[1024] = { 0 };
    unsigned long startTime = GetTickCount();
    int result = 0;
    while ((GetTickCount() - startTime) < 5000) {  // 超时5秒
        result = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);  // 留一位存放'\0'
        if (result > 0) {
            buffer[result] = '\0';
            return std::string(buffer);
        }
        else if (result == 0) {
            std::cout << "Server closed the connection." << std::endl;
            break;
        }
        else {
            int errorCode = WSAGetLastError();
            if (errorCode == WSAEWOULDBLOCK) {
                continue;
            }
            std::cout << "Error receiving data, error code: " << errorCode << std::endl;
            break;
        }
    }
    std::cout << "Timeout or error receiving data." << std::endl;
    return "";
}

void TCPClient::disconnect() {
    if (clientSocket != INVALID_SOCKET) {
        closesocket(clientSocket);
        clientSocket = INVALID_SOCKET;
    }
}
