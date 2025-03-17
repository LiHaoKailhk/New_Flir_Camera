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
    std::string chosen_trigger;
    double exposure_time;
    std::string serial_port;
    int baud_rate;
    std::string chosen_operation;
    std::string left_camera_id;
    std::string right_camera_id;
    int Calibration_image_number;
    int HeartImg_image_number;

private:
    nlohmann::json _json_config;
};