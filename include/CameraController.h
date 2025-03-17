// CameraController.h
#pragma once
#pragma execution_character_set("utf-8")
#include <Spinnaker.h>
#include <SpinGenApi/SpinnakerGenApi.h>
#include <opencv2/opencv.hpp>
#include <vector>
#include "Config.h"

using namespace Spinnaker;
using namespace Spinnaker::GenApi;
using namespace Spinnaker::GenICam;
using namespace std;

class CameraController {
public:
    CameraController(Config config);
    ~CameraController();

    Spinnaker::CameraList& GetCameras() { return _cameras; }
    bool InitializeCameras(CameraList& cameras);
    bool ConfigureCamera(CameraList& cameras);
    bool ResetCamera(CameraList& cameras);

private:
    SystemPtr _system;
    CameraList _cameras;
    Config _config;
    bool ConfigureTrigger(CameraPtr camera);
    bool ConfigureExposure(CameraPtr camera);
    bool ConfigureGain(CameraPtr camera);
    bool ConfigureAcquisitionMode(CameraPtr camera);
};