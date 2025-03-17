#include "SerialPort.h"
#include <iostream>
#include <stdexcept>
#include <string>

SerialPort::SerialPort(const std::string& portName, int baudRate /*= 9600*/) {
    // 使用 CreateFileA 支持 std::string
    _hSerial = CreateFileA(
        portName.c_str(),            // 串口名称（如 "COM1"）
        GENERIC_READ | GENERIC_WRITE,// 读写权限
        0,                           // 不共享
        0,                           // 默认安全属性
        OPEN_EXISTING,               // 打开已存在的串口
        FILE_ATTRIBUTE_NORMAL,       // 默认属性
        0                            // 不使用模板
    );

    if (_hSerial == INVALID_HANDLE_VALUE) {
        error("无法打开串口");
    }

    setup(baudRate);
}

SerialPort::~SerialPort() {
    if (_hSerial != INVALID_HANDLE_VALUE) {
        CloseHandle(_hSerial);
    }
}

void SerialPort::sendFrequency(int frequency) {
    std::string frequencyStr = std::to_string(frequency) + "\n";
    DWORD bytesWritten;

    if (!WriteFile(_hSerial, frequencyStr.c_str(), frequencyStr.size(), &bytesWritten, nullptr)) {
        error("写入串口失败");
    }
}

std::string SerialPort::readData() {
    const size_t bufferSize = 128;
    char buffer[bufferSize];
    DWORD bytesRead;
    std::string result;

    while (true) {
        if (!ReadFile(_hSerial, buffer, bufferSize - 1, &bytesRead, nullptr)) {
            break;
        }

        if (bytesRead == 0) {
            break;
        }

        buffer[bytesRead] = '\0';
        result += buffer;
    }

    return result;
}

void SerialPort::release() {
    if (_hSerial != INVALID_HANDLE_VALUE) {
        CloseHandle(_hSerial);
        _hSerial = INVALID_HANDLE_VALUE;
    }
}   

void SerialPort::setup(int baudRate) {
    DCB dcbSerialParams = { 0 };
    COMMTIMEOUTS timeouts = { 0 };

    dcbSerialParams.DCBlength = sizeof(dcbSerialParams);
    if (!GetCommState(_hSerial, &dcbSerialParams)) {
        error("获取串口状态失败");
    }

    dcbSerialParams.BaudRate = baudRate;
    dcbSerialParams.ByteSize = 8;
    dcbSerialParams.StopBits = ONESTOPBIT;
    dcbSerialParams.Parity = NOPARITY;

    if (!SetCommState(_hSerial, &dcbSerialParams)) {
        error("设置串口状态失败");
    }

    timeouts.ReadIntervalTimeout = 50;
    timeouts.ReadTotalTimeoutConstant = 200;

    if (!SetCommTimeouts(_hSerial, &timeouts)) {
        error("设置串口超时失败");
    }
}

void SerialPort::error(const std::string& message) {
    DWORD lastError = GetLastError();
    std::string errorMessage = message + " (错误代码: " + std::to_string(lastError) + ")";
    throw std::runtime_error(errorMessage);
}