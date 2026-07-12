#pragma once

#include <string>

/*
 * DiskCollector 负责采集磁盘使用率
 *数据来源于系统调用 statvfs()
 */

class DiskCollector
{
	public:
		bool collect(const std::string& path, double& disk_usage);
};
