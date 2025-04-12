#pragma once
#include <string>
#include <nlohmann/json.hpp>

class Config {
public:
    Config(const std::string& config_file);
    bool load_config(const std::string& config_file);
    const Config& get_camera_config() const { return *this; }

    // 配置参数
    std::string file_path;
    double exposure_time;
    std::string chosen_operation;
    std::string left_camera_id;
    std::string right_camera_id;
    int Calibration_image_number;
    int Acquisition_image_number;

    //采集频率
    int Acquisition_Hz;
    int Calibration_Hz;

    //相机触发相关
    std::string chosen_trigger;
    bool Use_WIFI_Trigger;
    std::string serial_port;
    int baud_rate;

private:
    nlohmann::json _json_config;
};