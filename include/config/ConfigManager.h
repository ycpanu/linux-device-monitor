#pragma once

#include <string>

/*
 * AppConfig 表示程序运行配置，
 *
 * 1.device_id：设备ID
 * 2.collect_interval：采集周期
 * 3.disk_path：磁盘监控路径
 */

struct AppConfig
{
	std::string device_id = "linux_gateway_001";
	int collect_interval = 5;
	std::string disk_path ="/";
};

/*
 *ConfigManager 负责加载 config/config.json.
 * 如果配置文件不存在，使用默认的配置，
 * 如果配置项缺失，也使用默认值。
 */

class ConfigManager
{
	public:
		bool load(const std::string& config_path);
		const AppConfig& getConfig() const;
	
	private:
		AppConfig config_;
};
