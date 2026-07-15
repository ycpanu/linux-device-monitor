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
    std::string database_path="../data/monitor.db";

    int alarm_continuous_count = 3;

    double cpu_warning_threshold = 70.0;
    double cpu_critical_threshold = 90.0;

    double memory_warning_threshold = 70.0;
    double memory_critical_threshold = 90.0;

    double disk_warning_threshold = 80.0;
    double disk_critical_threshold = 95.0;

    //MQTT配置
    bool mqtt_enabled = false;
    std::string mqtt_host = "localhost";
    int mqtt_port = 1883;
    std::string mqtt_client_id = "linux_gateway_001_agent";
    std::string mqtt_topic_prefix = "device";
    int mqtt_keepalive = 60;
    int mqtt_heartbeat_interval = 30;
};

class ConfigManager {
public:
    bool load(const std::string& config_path);
    const AppConfig& getConfig() const;

private:
    AppConfig config_;
};
