#include "config/ConfigManager.h"
#include "common/Logger.h"

#include <fstream>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

bool ConfigManager::load(const std::string& config_path) {
    std::ifstream file(config_path);

    if (!file.is_open()) {
        Logger::warn("config", "config file not found, use default config: " + config_path);
        return true;
    }

    try {
        json j;
        file >> j;

        config_.device_id = j.value("device_id", config_.device_id);
        config_.collect_interval = j.value("collect_interval", config_.collect_interval);
        config_.disk_path = j.value("disk_path", config_.disk_path);
 	config_.database_path = j.value("database_path", config_.database_path);
        if (config_.collect_interval <= 0) {
            Logger::warn("config", "invalid collect_interval, reset to 5 seconds");
            config_.collect_interval = 5;
        }

        /*
         * 读取 alarm 配置。
         *
         * contains("alarm")：判断 JSON 里是否存在 alarm 字段。
         * is_object()：判断 alarm 是否是一个 JSON 对象。
         */
        if (j.contains("alarm") && j["alarm"].is_object()) {
            json alarm = j["alarm"];

            config_.alarm_continuous_count =
                alarm.value("continuous_count", config_.alarm_continuous_count);

            config_.cpu_warning_threshold =
                alarm.value("cpu_warning_threshold", config_.cpu_warning_threshold);
            config_.cpu_critical_threshold =
                alarm.value("cpu_critical_threshold", config_.cpu_critical_threshold);

            config_.memory_warning_threshold =
                alarm.value("memory_warning_threshold", config_.memory_warning_threshold);
            config_.memory_critical_threshold =
                alarm.value("memory_critical_threshold", config_.memory_critical_threshold);

            config_.disk_warning_threshold =
                alarm.value("disk_warning_threshold", config_.disk_warning_threshold);
            config_.disk_critical_threshold =
                alarm.value("disk_critical_threshold", config_.disk_critical_threshold);
        }

        if (config_.alarm_continuous_count <= 0) {
            Logger::warn("config", "invalid alarm continuous_count, reset to 3");
            config_.alarm_continuous_count = 3;
        }

        Logger::info("config", "config loaded successfully");
        return true;
    } catch (const std::exception& e) {
        Logger::error("config", std::string("parse config failed: ") + e.what());
        return false;
    }
}

const AppConfig& ConfigManager::getConfig() const {
    return config_;
}
