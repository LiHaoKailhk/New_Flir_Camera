#pragma once
#include <string>
#include <windows.h>

class SerialPort {
public:
    SerialPort(const std::string& portName, int baudRate = 9600);
    ~SerialPort();

    void sendFrequency(int frequency);
    std::string readData();
    //释放这个串口
    void release(); 

private:
    HANDLE _hSerial;
    void setup(int baudRate);
    void error(const std::string& message);
};