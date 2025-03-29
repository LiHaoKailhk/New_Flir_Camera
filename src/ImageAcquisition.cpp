#pragma once
#include "ImageAcquisition.h"
#include "CameraController.h"
#include "SerialPort.h"
#include <Spinnaker.h>
#include <opencv2/opencv.hpp>
#include <iostream>
#include <vector>
#include <string>
#include <ctime>
#include <thread>
#include"direct.h"

using namespace Spinnaker;
using namespace cv;
using namespace std;

// 初始化图像采集类
// @param config: 相机配置参数
ImageAcquisition::ImageAcquisition(const Config& config)
    : _config(config),
      _isRunning(false),
      _cameraController(config),
      _acquisitionMode(AcquisitionMode::SINGLE_BATCH),
      _intervalSeconds(60)  // 默认1分钟
{
    SetImageNumber();
}


// 图像采集类析构函数
ImageAcquisition::~ImageAcquisition() {}

bool ImageAcquisition::SingleAcquisitionByTrigger(CameraPtr pCam, ImagePtr& pResultImage)
{
    try {
        // 如果是软触发模式，执行软触发
        if (_config.chosen_trigger == "SOFTWARE") {
            // Get user input
            std::cout << "Press the Enter key to initiate software trigger." << endl;
            getchar();

            // Execute software trigger
            if (!IsWritable(pCam->TriggerSoftware))
            {
                std::cout << "Unable to execute trigger..." << endl;
                return false;  // 修改为返回false
            }

            pCam->TriggerSoftware.Execute();
        } else {
            // 如果不是软触发模式，直接返回false
            std::cout << "HEARD TRIGGER" << endl;
        }

        // 获取下一帧图像
        pResultImage = pCam->GetNextImage();
        
        return true;
    }
    catch (Spinnaker::Exception& e) {
        std::cout << "trigger采集错误: " << e.what() << endl;
        return false;
    }
}

bool ImageAcquisition::SingleAcquisition_RAM(CameraPtr pCam)
{
    try {
        // 获取相机序列号，判断左右相机
        string cameraType ;
        if (pCam->DeviceSerialNumber.GetValue() == _config.left_camera_id) {
            cameraType = "Left";
        } else if (pCam->DeviceSerialNumber.GetValue() == _config.right_camera_id) {
            cameraType = "Right";
        } else {
            std::cout << "相机序列号不匹配" << endl;
            return false;
        }

        // 开始计时
        clock_t start_time = clock();

        std::cout << "目标采集张数： " << _image_number << std::endl;

        // 采集指定数量的图像
        for (unsigned int i = 0; i < _image_number; i++) {

            if (i == 1)
            {
                start_time = clock();
            }

            ImagePtr pResultImage = nullptr;
            bool result = SingleAcquisitionByTrigger(pCam, pResultImage);
            cout << i << endl;

            if (i == 0) {
                cout << "图像宽度: " << pResultImage->GetWidth() << endl;
                cout << "图像高度: " << pResultImage->GetHeight() << endl;
            }

            if (pResultImage->IsIncomplete()) {
                cout << "图像不完整: " << pResultImage->GetImageStatus() << endl;
                pResultImage->Release();
                continue;
            }

            // 转换并保存图像
            const size_t width = pResultImage->GetWidth();
            const size_t height = pResultImage->GetHeight();
            ImagePtr grayImage = pResultImage->Convert(PixelFormat_Mono8);
            Mat image = Mat(height, width, CV_8UC1, grayImage->GetData()).clone();

            if (cameraType == "Left") {
                _leftImages.push_back(image);
            } else {
                _rightImages.push_back(image);
            }

            
            pResultImage->Release();
        }

        // 输出采集时间
        clock_t end_time = clock();
        cout << "采集用时: " << (double)(end_time - start_time) / CLOCKS_PER_SEC << "s" << endl;

        // 结束采集
        pCam->EndAcquisition();

        return true;
    }
    catch (Spinnaker::Exception& e) {
        cout << "采集错误: " << e.what() << endl;
        return false;
    }
}

// MultipleAcquisition的实现可以使用OpenMP进行并行处理
bool ImageAcquisition::MultipleAcquisition(CameraList cameras)
{
    try {
        SetImageNumber();
        int camListSize = cameras.GetSize();      
        SerialPort serialPort(_config.serial_port, _config.baud_rate);
        serialPort.sendFrequency(0);
        Sleep(500);
        string result;

#pragma omp parallel for 
        for (int i = 0; i < camListSize + 1; i++) {
            if (i < camListSize) {
                SingleAcquisition_RAM(cameras.GetByIndex(i));
            } else {
                Sleep(1000);
                if (_config.chosen_operation == "Calibration") {
                    serialPort.sendFrequency(_config.Calibration_acquisition_Hz); // 发送2Hz的频率
                } else if (_config.chosen_operation == "HeartImg") {
                    serialPort.sendFrequency(_config.Image_acquisition_Hz); // 发送50Hz的频率
                }
                Sleep(200);
                string _result = serialPort.readData();
                cout << _result << endl;
            }
        }

        serialPort.sendFrequency(0);
        Sleep(200);
        result = serialPort.readData();
        cout << result << endl;

        return true;
    }
    catch (Spinnaker::Exception& e) {
        cout << "多相机采集错误: " << e.what() << endl;
        return false;
    }
}

// 添加停止采集的方法
void ImageAcquisition::StopAcquisition() {
    _isRunning = false;
}

bool ImageAcquisition::ContinuousAcquisition(CameraList cameras) {
    try {
        _isRunning = true;
        _leftImages.clear();
        _rightImages.clear();
        std::thread saveThread;  // 用于保存图像的线程

        while (_isRunning) {
            // 多相机采集
            if (!MultipleAcquisition(cameras)) {
                std::cout << "采集失败！" << std::endl;
                break;
            }

            // 启动保存图像线程
            saveThread = std::thread([this, &cameras]() {
                saveImageToDisk();
                _cameraController.ResetCamera(cameras);
                _cameraController.InitializeCameras(cameras);
                _cameraController.ConfigureCamera(cameras);
                });


            // 等待时间间隔
            auto start_time = std::chrono::steady_clock::now();
            while (_isRunning) {
                auto current_time = std::chrono::steady_clock::now();
                auto elapsedTime = std::chrono::duration_cast<std::chrono::seconds>(current_time - start_time).count();
                if (elapsedTime >= static_cast<int>(_intervalSeconds)) {
                    break;
                }

                // 实时显示剩余时间
                int remainingSeconds = _intervalSeconds - elapsedTime;
                std::cout << "剩余时间: " << remainingSeconds << " 秒\r" << std::flush;

                // 睡眠一小段时间，减少CPU占用
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }

            // 等待保存图像线程完成
            if (saveThread.joinable()) {
                saveThread.join();
            }
        }

        return true;
    }
    catch (const Spinnaker::Exception& e) {
        std::cout << "连续采集错误: " << e.what() << std::endl;
        return false;
    }
}

    
void ImageAcquisition::saveImageToDisk() {
    assert(_leftImages.size() == _rightImages.size());

    //图像储存逻辑为：_config.file_path下Heart-Img文件夹内储存，每次采集后，在Heart-Img文件夹内创建一个以当前"日期-时间戳"命名的文件夹，将图像保存到该文件夹内
    //获得日期+小时+分钟+秒
    time_t now = time(0);
    tm *ltm = localtime(&now);
    string date = to_string(1900 + ltm->tm_year) + "-" + to_string(1 + ltm->tm_mon) + "-" + to_string(ltm->tm_mday);
    string time = to_string(ltm->tm_hour) + "-" + to_string(ltm->tm_min) + "-" + to_string(ltm->tm_sec);
    string number = to_string(_image_number);
    string folder_name;
    if (_config.chosen_operation == "Calibration") {
        folder_name = _config.file_path + "Calibration/" + date + "-" + time;
    } else if (_config.chosen_operation == "HeartImg") {
        folder_name = _config.file_path + "Heart-Img/" + date + "-" + time + "-" + number;
    }
    mkdir(folder_name.c_str());

    // 创建文件夹CameraL和CameraR
    string cameraL_folder = folder_name + "/CameraL";
    string cameraR_folder = folder_name + "/CameraR";
    mkdir(cameraL_folder.c_str());
    mkdir(cameraR_folder.c_str());


    for (int i = 0; i < _leftImages.size(); i++) {
        string filename = cameraL_folder + "/" + to_string(i) + ".bmp";
        imwrite(filename, _leftImages[i]);
    }
    for (int i = 0; i < _rightImages.size(); i++) {
        string filename = cameraR_folder + "/" + to_string(i) + ".bmp";
        imwrite(filename, _rightImages[i]);
    }
    cout << "图像保存完成" << endl;
    _leftImages.clear();
    _rightImages.clear();
}

void ImageAcquisition::SetImageNumber() {
    if (_config.chosen_operation == "Calibration") {
        _image_number = _config.Calibration_image_number;
    } else if (_config.chosen_operation == "HeartImg") {
        if (_acquisitionMode == AcquisitionMode::SINGLE_BATCH) {
            _image_number = _config.HeartImg_image_number;
        } else if (_acquisitionMode == AcquisitionMode::CONTINUOUS) {
            _image_number = int(_config.HeartImg_image_number / 2);         // 连续采集时，采集次数减半
        }
    }
}
