#include "common/Logger.h"
#include "config/ConfigManager.h"
#include "collector/SystemMetrics.h"
#include "collector/CpuCollector.h"
#include "collector/MemoryCollector.h"
#include "collector/DiskCollector.h"

#include <chrono>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <thread>

/*
 * 把 double 类型保留两位小数，转换成字符串。
 *
 * 例如：
 * 35.6789 -> "35.68"
 */
std::string formatDouble(double value) {
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(2) << value;
    return oss.str();
}

int main(int argc, char* argv[]) {
    std::string config_path = "config/config.json";

    /*
     * 支持通过命令行参数指定配置文件路径。
     *
     * 例如：
     * ./device-monitor -c config/config.json
     */
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
     * 主循环：
     * 每隔 collect_interval 秒采集一次系统状态。
     *
     * 第一阶段先使用单线程循环。
     * 后续如果要支持多模块并发，可以再改成多线程模型。
     */
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
        } else {
            Logger::warn("main", "some metrics collect failed");
        }

        /*
         * sleep_for 表示让当前线程睡眠一段时间。
         * 这里表示每隔 collect_interval 秒采集一次。
         */
        std::this_thread::sleep_for(std::chrono::seconds(config.collect_interval));
    }

    return 0;
}
