#include "collector/MemoryCollector.h"
#include "common/Logger.h"

#include <fstream>
#include <sstream>
#include <string>

bool MemoryCollector::collect(double& memory_usage) {
    std::ifstream file("/proc/meminfo");
    if (!file.is_open()) {
        Logger::error("collector", "failed to open /proc/meminfo");
        return false;
    }

    std::string line;
    long mem_total = 0;
    long mem_available = 0;

    /*
     * /proc/meminfo 每一行格式类似：
     * MemTotal:        8041232 kB
     * MemAvailable:    5123456 kB
     */
    while (std::getline(file, line)) {
        std::istringstream iss(line);

        std::string key;
        long value;
        std::string unit;

        iss >> key >> value >> unit;

        if (key == "MemTotal:") {
            mem_total = value;
        } else if (key == "MemAvailable:") {
            mem_available = value;
        }

        /*
         * 两个关键字段都读到了，就可以停止循环。
         */
        if (mem_total > 0 && mem_available > 0) {
            break;
        }
    }

    if (mem_total <= 0) {
        Logger::error("collector", "invalid MemTotal value");
        return false;
    }

    memory_usage =
        static_cast<double>(mem_total - mem_available) / mem_total * 100.0;

    return true;
}
