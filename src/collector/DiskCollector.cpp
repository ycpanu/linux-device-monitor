#include "collector/DiskCollector.h"
#include "common/Logger.h"

#include <sys/statvfs.h>

bool DiskCollector::collect(const std::string& path, double& disk_usage)
{
	struct statvfs fs_info{};

	if(statvfs(path.c_str(), &fs_info)!=0)
	{
		Logger::error("collector", "failed to get disk info for path:" + path);
		return false;
	}

	//f_blocks：文件系统总块数
	//f_bfree：空闲块数
	
	unsigned long total_blocks = fs_info.f_blocks;
	unsigned long free_blocks = fs_info.f_bfree;

	if(total_blocks == 0)
	{
		Logger::error("collector", "invalid disk total blocks");
		return false;
	}

	disk_usage = static_cast<double>(total_blocks - free_blocks) / total_blocks * 100.0;
	return true;
}

