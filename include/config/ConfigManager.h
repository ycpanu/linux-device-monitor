#pragma once

#include <string>

/*
 * AppConfig 表示程序运行配置。
 *
 * 第一阶段已有：
 * 1. device_id
 * 2. collect_interval
 * 3. disk_path
 *
 * 第二阶段新增：
 * 1. CPU 告警阈值
 * 2. 内存告警阈值
 * 3. 磁盘告警阈值
 * 4. 连续触发次数
 */
struct AppConfig {
    std::string device_id = "linux_gateway_001";
    int collect_interval = 5;
    std::string disk_path = "/";

    int alarm_continuous_count = 3;

    double cpu_warning_threshold = 70.0;
    double cpu_critical_threshold = 90.0;

    double memory_warning_threshold = 70.0;
    double memory_critical_threshold = 90.0;

    double disk_warning_threshold = 80.0;
    double disk_critical_threshold = 95.0;
};

class ConfigManager {
public:
    bool load(const std::string& config_path);
    const AppConfig& getConfig() const;

private:
    AppConfig config_;
};
