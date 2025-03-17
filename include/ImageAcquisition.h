// ImageAcquisition.h
#pragma once
#pragma execution_character_set("utf-8")
#include <Spinnaker.h>
#include <opencv2/opencv.hpp>
#include <vector>
#include "Config.h"
#include "CameraController.h"


using namespace Spinnaker;
using namespace cv;
using namespace std;

// 添加采集模式枚举
enum class AcquisitionMode {
    SINGLE_BATCH,    // 单次采集指定张数
    CONTINUOUS       // 连续采集模式，定时采集
};

class ImageAcquisition {
public:
    ImageAcquisition(const Config& config);
    ~ImageAcquisition();

    // 添加新的成员变量
    bool _isRunning;
    AcquisitionMode _acquisitionMode;
    int _intervalSeconds;  // 连续采集模式下的时间间隔（秒）

    // 单帧图像采集
    bool SingleAcquisitionByTrigger(CameraPtr pCam, ImagePtr& pResultImage);
    // RAM模式采集（图像存储在内存中）
    bool SingleAcquisition_RAM(CameraPtr pCam);
    // 多相机并行采集
    bool MultipleAcquisition(CameraList cameras);
    // 连续采集
    bool ContinuousAcquisition(CameraList cameras);
    void saveImageToDisk();
    // 停止采集
    void StopAcquisition();

private:
    const Config& _config;
    int _image_number;
    vector<Mat> _leftImages;
    vector<Mat> _rightImages;
    void SetImageNumber();
    CameraController _cameraController;
};