#pragma once
#ifndef TCPCLIENT_H
#define TCPCLIENT_H

#include <winsock2.h>
#include <ws2tcpip.h>
#include <iostream>
#include <stdexcept>
#include <string>

#pragma comment(lib, "ws2_32.lib")

class TCPClient {
public:
    TCPClient(const std::string& serverIP, int serverPort);
    ~TCPClient();

    void connectToServer();
    void sendData(const std::string& data);
    std::string receiveData();
    void disconnect();

private:
    std::string serverIP;
    int serverPort;
    SOCKET clientSocket;
    sockaddr_in serverAddr;
};

#endif // TCPCLIENT_H