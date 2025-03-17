#include "SerialPort.h"
#include <iostream>
#include <string>

int main1() {
    try {
        std::string portName;
        int baudRate;

        SerialPort port("COM4", 9600);
        std::cout << "串口已成功打开: " << portName << std::endl;

        while (true) {
            std::string input;
            std::cout << "请输入要发送的数据（输入 'exit' 退出）: ";
            std::cin >> input;

            if (input == "exit") {
                break;
            }

            port.sendFrequency(std::stoi(input)); // 假设发送的是整数频率
            std::string response = port.readData();
            std::cout << "从串口接收到的数据: " << response << std::endl;
        }

        std::cout << "程序已退出。" << std::endl;
    }
    catch (const std::exception& e) {
        std::cerr << "发生错误: " << e.what() << std::endl;
        return 1;
    }

    system("pause");

    return 0;
}