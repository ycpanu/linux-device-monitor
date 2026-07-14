#include "common/Logger.h"
#include "config/ConfigManager.h"
#include "collector/SystemMetrics.h"
#include "collector/CpuCollector.h"
#include "collector/MemoryCollector.h"
#include "collector/DiskCollector.h"
#include "alarm/AlarmEngine.h"

#include <chrono>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <thread>

std::string formatDouble(double value) {
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(2) << value;
    return oss.str();
}

int main(int argc, char* argv[]) {
    std::string config_path = "config/config.json";

    if (argc == 3 && std::string(argv[1]) == "-c") {
        config_path = argv[2];
    }

    Logger::info("main", "Linux Device Monitor Agent started");

    ConfigManager config_manager;
    if (!config_manager.load(config_path)) {
        Logger::error("main", "failed to load config, program exit");
        return 1;
    }

    AppConfig config = config_manager.getConfig();

    Logger::info("main", "device_id: " + config.device_id);
    Logger::info("main", "collect_interval: " + std::to_string(config.collect_interval) + " seconds");
    Logger::info("main", "disk_path: " + config.disk_path);

    CpuCollector cpu_collector;
    MemoryCollector memory_collector;
    DiskCollector disk_collector;

    /*
     * 根据配置文件创建告警规则。
     */
    AlarmRule alarm_rule;
    alarm_rule.continuous_count = config.alarm_continuous_count;

    alarm_rule.cpu_warning_threshold = config.cpu_warning_threshold;
    alarm_rule.cpu_critical_threshold = config.cpu_critical_threshold;

    alarm_rule.memory_warning_threshold = config.memory_warning_threshold;
    alarm_rule.memory_critical_threshold = config.memory_critical_threshold;

    alarm_rule.disk_warning_threshold = config.disk_warning_threshold;
    alarm_rule.disk_critical_threshold = config.disk_critical_threshold;

    AlarmEngine alarm_engine(alarm_rule);

    while (true) {
        SystemMetrics metrics;
        metrics.disk_path = config.disk_path;

        bool cpu_ok = cpu_collector.collect(metrics.cpu_usage);
        bool mem_ok = memory_collector.collect(metrics.memory_usage);
        bool disk_ok = disk_collector.collect(config.disk_path, metrics.disk_usage);

        if (cpu_ok && mem_ok && disk_ok) {
            std::cout << "----------------------------------------" << std::endl;
            std::cout << "Device ID    : " << config.device_id << std::endl;
            std::cout << "CPU Usage    : " << formatDouble(metrics.cpu_usage) << "%" << std::endl;
            std::cout << "Memory Usage : " << formatDouble(metrics.memory_usage) << "%" << std::endl;
            std::cout << "Disk Usage   : " << formatDouble(metrics.disk_usage) << "%" << std::endl;
            std::cout << "Disk Path    : " << metrics.disk_path << std::endl;

            /*
             * 第二阶段新增：
             * 将采集到的系统指标交给告警引擎判断。
             */
            std::vector<AlarmEvent> alarm_events = alarm_engine.checkMetrics(metrics);

            for (const auto& event : alarm_events) {
                if (event.recovered) {
                    Logger::info("alarm", "[" + event.alarm_type + "] " + event.message);
                } else if (event.level == "CRITICAL") {
                    Logger::error("alarm", "[" + event.alarm_type + "] " + event.message);
                } else {
                    Logger::warn("alarm", "[" + event.alarm_type + "] " + event.message);
                }
            }
        } else {
            Logger::warn("main", "some metrics collect failed");
        }

        std::this_thread::sleep_for(std::chrono::seconds(config.collect_interval));
    }

    return 0;
}
