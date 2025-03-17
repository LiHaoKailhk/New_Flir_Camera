#include "Config.h"
#include <iostream>
#include <fstream>
#include <stdexcept>
#include <filesystem>
#include <nlohmann/json.hpp>

using namespace std;
using json = nlohmann::json;

Config::Config(const string& config_file) {
    load_config(config_file);
}

bool Config::load_config(const string& config_file) {
    try {
        ifstream file(config_file);
        if (!file.is_open()) {
            cerr << "无法打开配置文件: " << config_file << endl;
            return false;
        }

        file >> _json_config;

        // 读取配置
        file_path = _json_config["file_path"];
        chosen_trigger = _json_config["chosen_trigger"];
        exposure_time = _json_config["exposure_time"];
        serial_port = _json_config["serial_port"];
        baud_rate = _json_config["baud_rate"];
        chosen_operation = _json_config["chosen_operation"];
        left_camera_id = _json_config["left_camera_id"];
        right_camera_id = _json_config["right_camera_id"];
        Calibration_image_number = _json_config["Calibration_image_number"];
        HeartImg_image_number = _json_config["HeartImg_image_number"];

        // 检查路径是否完整存在
        //如果路径不存在，则创建路径
        //file_path
        //file_path/Heart-Img
        //file_path/Calibration
        if (!filesystem::exists(file_path)) {
            cerr << "文件路径不存在: " << file_path << endl;
            //如果路径不存在，则创建路径
            filesystem::create_directories(file_path);
            //在文件路径下创建Heart-Img文件夹
            filesystem::create_directories(file_path + "/Heart-Img");
            //在Heart-Img文件夹下创建Calibration文件夹
            filesystem::create_directories(file_path + "/Heart-Img/Calibration");
        }

        return true;
    } catch (const exception& e) {
        cerr << "加载配置文件失败：" << e.what() << endl;
        return false;
    }
}