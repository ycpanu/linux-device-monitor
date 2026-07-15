#include "common/Logger.h"
#include "config/ConfigManager.h"
#include "collector/SystemMetrics.h"
#include "collector/CpuCollector.h"
#include "collector/MemoryCollector.h"
#include "collector/DiskCollector.h"
#include "alarm/AlarmEngine.h"
#include "storage/SQLiteStorage.h"
#include "communication/MqttReporter.h"

#include <chrono>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <thread>
#include <ctime>

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
    Logger::info("main", "database_path:" + config.database_path);
    Logger::info("main", "mqtt_enabled:" + std::string(config.mqtt_enabled ? "true" : "false"));
    Logger::info("main", "mqtt_host:" + config.mqtt_host);
    Logger::info("main", "mqtt_port:" + std::to_string(config.mqtt_port));

    CpuCollector cpu_collector;
    MemoryCollector memory_collector;
    DiskCollector disk_collector;

    SQLiteStorage storage;
    if(!storage.init(config.database_path))
    {
	    Logger::error("main", "failed to initialize storage, program exit");
	    return 1;
    }

    //初始化mqtt
    MqttConfig mqtt_config;
    mqtt_config.enabled = config.mqtt_enabled;
    mqtt_config.host = config.mqtt_host;
    mqtt_config.port = config.mqtt_port;
    mqtt_config.client_id = config.mqtt_client_id;
    mqtt_config.topic_prefix = config.mqtt_topic_prefix;
    mqtt_config.keepalive = config.mqtt_keepalive;

    MqttReporter mqtt_reporter;
    if(!mqtt_reporter.init(mqtt_config))
    {
	    Logger::warn("main", "mqtt reporter init falied, continue running locally");
    }

    long last_heartbeat_time = 0;


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
	metrics.timestamp = std::time(nullptr);

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

	    if(!storage.saveMetrics(config.device_id,metrics))
	    {
		    Logger::warn("main", "save metrics failed");
	    }

	    if(!mqtt_reporter.reportMetrics(config.device_id, metrics))
	    {
		    Logger::warn("main", "report metrics falied");
	    }

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

		if(!storage.saveAlarm(config.device_id, event))
		{
			Logger::warn("main", "save alarm failed");
		}

		if(!mqtt_reporter.reportAlarm(config.device_id, event))
		{
			Logger::warn("main", "report alarm falied");
		}
            }
        } else {
            Logger::warn("main", "some metrics collect failed");
        }

	long now = std::time(nullptr);
	if(now - last_heartbeat_time >= config.mqtt_heartbeat_interval)
	{
		if(!mqtt_reporter.reportHeartbeat(config.device_id))
		{
			Logger::warn("main", "report heartbeat falied");
		}
		last_heartbeat_time = now;
	}

        std::this_thread::sleep_for(std::chrono::seconds(config.collect_interval));
    }

    return 0;
}
