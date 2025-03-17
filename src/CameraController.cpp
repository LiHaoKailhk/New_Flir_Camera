#include "CameraController.h"
#include <Spinnaker.h>
#include <SpinGenApi/SpinnakerGenApi.h>
#include <iostream>
#include <string>

using namespace Spinnaker;
using namespace Spinnaker::GenApi;
using namespace Spinnaker::GenICam;
using namespace std;

// 相机控制器类的构造函数
// @param config: 配置对象
CameraController::CameraController(Config config) 
    : _config(config) {
    // 初始化 Spinnaker
    _system = System::GetInstance();
    _cameras = _system->GetCameras();
    _config = config;
}

// 相机控制器类的析构函数
// 释放相机系统资源
CameraController::~CameraController() {
    _cameras.Clear();
    _system->ReleaseInstance();
}

// 初始化相机列表
// @param cameras: 相机列表对象
// @return 是否初始化成功
bool CameraController::InitializeCameras(CameraList& cameras) {
    try {
        // 获取相机列表
        cameras = _system->GetCameras();
        // 初始化每台相机
        for (unsigned int i = 0; i < cameras.GetSize(); ++i) {
            CameraPtr camera = cameras.GetByIndex(i);
            camera->Init();
        }
        return true;
    }
    catch (Spinnaker::Exception& e) {
        cout << "相机初始化失败: " << e.what() << endl;
        return false;
    }
}

// 配置相机参数
// @param camera: 相机对象指针
// @return 是否配置成功
bool CameraController::ConfigureCamera(CameraList& cameras) {
    bool result = true;

    for (unsigned int i = 0; i < cameras.GetSize(); ++i) {
        CameraPtr camera = cameras.GetByIndex(i);
        // 配置触发模式
        result &= ConfigureTrigger(camera);
        // 配置曝光时间
        result &= ConfigureExposure(camera);
        // 配置增益
        result &= ConfigureGain(camera);
        // 配置采集模式
        result &= ConfigureAcquisitionMode(camera);
        cameras.GetByIndex(i)->DeviceLinkThroughputLimit.SetValue(500000000);
    }   
    return result;
}

bool CameraController::ResetCamera(CameraList& cameras)
{
   string Trigger = "SOFTWARE";
    bool result = true;

    cout << endl << endl << "***RESET CAMERA***" << endl << endl;

    for (int i = 0; i < cameras.GetSize(); i++)
    {
        CameraPtr pCam = cameras.GetByIndex(i);

        try
        {
            pCam->TriggerMode.SetValue(TriggerMode_Off);

            if (!IsReadable(pCam->TriggerSource) || !IsWritable(pCam->TriggerSource))
            {
                cout << "Unable to set trigger mode (node retrieval).Aborting..." << endl;
                return -1;
            }
            pCam->TriggerSource.SetValue(TriggerSource_Software);
            cout << "Trigger source set to software..." << endl;
        }
        catch (Spinnaker::Exception& e)
        {
            cout << "Error:" << e.what() << endl;
            result = -1;
        }
    }
    return false;

}

// 配置触发模式
// @param pCam: 相机对象指针
// @return 是否配置成功
bool CameraController::ConfigureTrigger(CameraPtr pCam) {
    bool result = true;

    try {
        // 选择触发模式
        if (_config.chosen_trigger == "SOFTWARE") {
            cout << "已选择软件触发..." << endl;
        }
        else {
            cout << "已选择硬件触发..." << endl;
        }

        // 禁用触发模式
        if (!IsReadable(pCam->TriggerMode) || !IsWritable(pCam->TriggerMode)) {
            cout << "无法禁用触发模式。操作失败..." << endl;
            return false;
        }

        pCam->TriggerMode.SetValue(TriggerMode_Off);
        cout << "触发模式已禁用..." << endl;

        // 设置触发选择器为帧启动
        if (!IsReadable(pCam->TriggerSelector) || !IsWritable(pCam->TriggerSelector)) {
            cout << "无法设置触发选择器（节点检索失败）。操作失败..." << endl;
            return false;
        }

        pCam->TriggerSelector.SetValue(TriggerSelector_FrameStart);
        cout << "触发选择器已设置为帧启动..." << endl;

        // 设置触发重叠
        pCam->TriggerOverlap.SetValue(TriggerOverlap_ReadOut);

        // 设置触发源
        if (!IsReadable(pCam->TriggerSource) || !IsWritable(pCam->TriggerSource)) {
            cout << "无法设置触发源（节点检索失败）。操作失败..." << endl;
            return false;
        }

        if (_config.chosen_trigger == "SOFTWARE") {
            pCam->TriggerSource.SetValue(TriggerSource_Software);
            cout << "触发源已设置为软件..." << endl;
        }
        else {
            pCam->TriggerSource.SetValue(TriggerSource_Line0);
            cout << "触发源已设置为硬件..." << endl;
        }

        // 启用触发模式
        if (!IsReadable(pCam->TriggerMode) || !IsWritable(pCam->TriggerMode)) {
            cout << "无法启用触发模式。操作失败..." << endl;
            return false;
        }

        pCam->TriggerMode.SetValue(TriggerMode_On);
        cout << "触发模式已启用..." << endl << endl;
    }
    catch (Spinnaker::Exception& e) {
        cout << "触发模式配置失败: " << e.what() << endl;
        result = false;
    }

    return result;
}

// 配置曝光时间
// @param pCam: 相机对象指针
// @return 是否配置成功
bool CameraController::ConfigureExposure(CameraPtr pCam) {
    bool result = true;

    try {
        // 禁用自动曝光
        if (!IsReadable(pCam->ExposureAuto) || !IsWritable(pCam->ExposureAuto)) {
            cout << "无法禁用自动曝光。操作失败..." << endl;
            return false;
        }

        pCam->ExposureAuto.SetValue(ExposureAuto_Off);
        cout << "自动曝光已禁用..." << endl;

        // 设置曝光时间
        if (!IsReadable(pCam->ExposureTime) || !IsWritable(pCam->ExposureTime)) {
            cout << "无法设置曝光时间。操作失败..." << endl;
            return false;
        }

        pCam->ExposureMode.SetValue(ExposureMode_Timed);
        pCam->ExposureTime.SetValue(_config.exposure_time);
        cout << "快门时间已设置为 " << _config.exposure_time << " 微秒..." << endl;
    }
    catch (Spinnaker::Exception& e) {
        cout << "曝光时间配置失败: " << e.what() << endl;
        result = false;
    }

    return result;
}

// 配置增益
// @param pCam: 相机对象指针
// @return 是否配置成功
bool CameraController::ConfigureGain(CameraPtr pCam) {
    bool result = true;

    try {
        pCam->GainAuto.SetValue(GainAuto_Once);
        cout << "增益自动设置为一次..." << endl;
    }
    catch (Spinnaker::Exception& e) {
        cout << "增益配置失败: " << e.what() << endl;
        result = false;
    }

    return result;
}

// 配置采集模式
// @param pCam: 相机对象指针
// @return 是否配置成功
bool CameraController::ConfigureAcquisitionMode(CameraPtr pCam) {
    bool result = true;

    try {
        // 设置采集模式为连续采集
        if (!IsReadable(pCam->AcquisitionMode) || !IsWritable(pCam->AcquisitionMode)) {
            cout << "无法将采集模式设置为连续采集。操作失败..." << endl;
            return false;
        }

        pCam->AcquisitionMode.SetValue(AcquisitionMode_Continuous);
        cout << "采集模式已设置为连续采集..." << endl;

        // 配置其他设置
        ConfigureExposure(pCam);
        ConfigureTrigger(pCam);
        ConfigureGain(pCam);

        // 开始采集
        pCam->BeginAcquisition();
        cout << "采集已开始..." << endl;
    }
    catch (Spinnaker::Exception& e) {
        cout << "采集模式配置失败: " << e.what() << endl;
        result = false;
    }

    return result;
}