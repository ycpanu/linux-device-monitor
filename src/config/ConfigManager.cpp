#include "config/ConfigManager.h"
#include "common/Logger.h"

#include <fstream>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

bool ConfigManager::load(const std::string& config_path)
{
	std::ifstream file(config_path);

	/*
	 * 如果打不开，说明文件不存在或者没有权限读取
	 */

	if(!file.is_open())
	{
		Logger::warn("config","config file not found, use default config:"+config_path);
		return true;
	}

	try
	{
		json j;
		file >> j;
		
		/*
		 * value(“字段名”，默认值)
		 * 如果存在JSON中存在该字段就读取
		 * 不存在就使用默认值
		 */
		config_.device_id =  j.value("device_id",config_.device_id);
		config_.collect_interval = j.value("collect_interval",config_.collect_interval);
		config_.disk_path = j.value("disk_path",config_.disk_path);

		/*简单校验，避免采集周期配置成0或负值*/
		if(config_.collect_interval <= 0)
		{
			Logger::warn("config","invalid collect_interval,reset to 5 seconds");
			config_.collect_interval = 5;
		}

		Logger::info("config","config loaded successfully");
		return true;
	}catch(const std::exception& e){
		//JSON 格式错误
		Logger::error("config",std::string("parse config faild:")+e.what());
		return false;
	}
}

const AppConfig& ConfigManager::getConfig() const{
	return config_;
}

