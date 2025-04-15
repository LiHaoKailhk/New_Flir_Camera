#pragma once
#include <Spinnaker.h>
#include "CameraController.h"
#include "ImageAcquisition.h"
#include "SerialPort.h"
#include "Config.h"
#include <iostream>
#include <locale>
#include <thread>
#include <string>

using namespace std;

void showMenu(Config _config) {
    cout << "\n=== 图像采集系统 ===\n"
        << "   当前采集状态： " << _config.chosen_operation << "\n"
        << "   当前保存路径： " << _config.file_path << endl;
    cout << "1. 单次批量采集   (get "
        << _config.Acquisition_image_number
        << " frames by "
        << _config.Acquisition_Hz
        << " Hz)" << endl;
    cout << "2. 连续定时采集 (每次采集张数默认为一半）" << endl;
    cout << "3. 退出程序" << endl;

    cout << "请选择操作: ";
}

int main(int /*argc*/, char** /*argv*/) {

    try {
        // 初始化系统
        SystemPtr system = System::GetInstance();
        CameraList camList = system->GetCameras();

        unsigned int numCameras = camList.GetSize();
        
        if (camList.GetSize() < 2) {
            cout << "错误: 未检测到足够的相机" << endl;
            return camList.GetSize();
        }

        // 读取配置文件
        Config config(".\\config\\config.json");
        if (!config.load_config(".\\config\\config.json")) {
            cout << "加载配置文件失败" << endl;
            std::system("pause");
            return -1;
        }

        if (!config.Use_WIFI_Trigger) {
            cout << config.Use_WIFI_Trigger << endl;
            SerialPort serialPort(config.serial_port, config.baud_rate);
            serialPort.sendFrequency(0);
            string respone = serialPort.readData();
            cout << respone << endl;
            serialPort.release();
        }

        //初始化相机
        CameraController cameraController(config);
        cameraController.InitializeCameras(camList);
        cameraController.ConfigureCamera(camList);

        // 创建图像采集对象
        ImageAcquisition imageAcquisition(config);
        
        int choice;
        while (true) {
            showMenu(config);
            cin >> choice;
            
            switch (choice) {
                case 1: {
                    // 单次批量采
                    cout << "执行单次批量采集..." << endl;
                    imageAcquisition._acquisitionMode = AcquisitionMode::SINGLE_BATCH;
                    if (imageAcquisition.MultipleAcquisition(camList)) {
                        cout << "采集完成" << endl;
                        imageAcquisition.saveImageToDisk();
                        cameraController.ResetCamera(camList);
                        cameraController.InitializeCameras(camList);
                        cameraController.ConfigureCamera(camList);
                    } else {
                        cout << "采集失败" << endl;
                    }
                    break;
                }
                
                case 2: {
                    // 连续定时采集
                    cout << "请输入采集间隔时间(秒) 默认1分钟: "; 
                    int interval;
                    cin >> interval;
                    if (interval == 0) {
                        interval = 60;
                    }
                    imageAcquisition._intervalSeconds = interval;
                    imageAcquisition._acquisitionMode = AcquisitionMode::CONTINUOUS;
                    
                    cout << "开始连续采集(按回车键停止)..." << endl;
                    
                    // 创建采集线程
                    thread acquisitionThread(&ImageAcquisition::ContinuousAcquisition, 
                                          &imageAcquisition, camList);
                    
                    // 等待用户按回车停止
                    cin.ignore(numeric_limits<streamsize>::max(), '\n');
                    cin.get();
                    
                    // 停止采集并等待线程结束
                    imageAcquisition.StopAcquisition();
                    acquisitionThread.join();
                    cout << "连续采集已停止" << endl;
                    break;
                }
                
                case 3: {
                    cout << "程序退出" << endl;
                    camList.Clear();
                    system->ReleaseInstance();
                    return 0;
                }
                
                default: {
                    cout << "无效的选择，请重试" << endl;
                    break;
                }
            }
        }
    }
    catch (Spinnaker::Exception& e) {
        cout << "错误: " << e.what() << endl;
        return -1;
    }
    
    return 0;
}