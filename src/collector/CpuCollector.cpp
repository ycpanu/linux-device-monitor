#include "collector/CpuCollector.h"
#include "common/Logger.h"

#include <fstream>
#include <sstream>

CpuCollector::CpuCollector() = default;

/*
 * 从 /proc/stat 读取第一行 cpu 数据。
 *
 * /proc/stat 第一行示例：
 * cpu  123 0 456 789 10 0 20 0 0 0
 */
bool CpuCollector::readCpuTimes(CpuTimes& times) {
    std::ifstream file("/proc/stat");
    if (!file.is_open()) {
        Logger::error("collector", "failed to open /proc/stat");
        return false;
    }

    std::string line;
    std::getline(file, line);

    std::istringstream iss(line);
    std::string cpu_label;

    iss >> cpu_label
        >> times.user
        >> times.nice
        >> times.system
        >> times.idle
        >> times.iowait
        >> times.irq
        >> times.softirq
        >> times.steal;

    if (cpu_label != "cpu") {
        Logger::error("collector", "invalid /proc/stat format");
        return false;
    }

    return true;
}

/*
 * 空闲时间 = idle + iowait
 */
uint64_t CpuCollector::getIdleTime(const CpuTimes& times) {
    return times.idle + times.iowait;
}

/*
 * 总时间 = 所有 CPU 状态时间之和
 */
uint64_t CpuCollector::getTotalTime(const CpuTimes& times) {
    return times.user
         + times.nice
         + times.system
         + times.idle
         + times.iowait
         + times.irq
         + times.softirq
         + times.steal;
}

bool CpuCollector::collect(double& cpu_usage) {
    CpuTimes current_times;

    if (!readCpuTimes(current_times)) {
        return false;
    }

    /*
     * 第一次采集时，没有上一次数据，无法计算差值。
     * 此时先保存当前值，返回 0。
     */
    if (!has_last_) {
        last_times_ = current_times;
        has_last_ = true;
        cpu_usage = 0.0;
        return true;
    }

    uint64_t last_idle = getIdleTime(last_times_);
    uint64_t current_idle = getIdleTime(current_times);

    uint64_t last_total = getTotalTime(last_times_);
    uint64_t current_total = getTotalTime(current_times);

    uint64_t total_delta = current_total - last_total;
    uint64_t idle_delta = current_idle - last_idle;

    /*
     * 防止除以 0。
     */
    if (total_delta == 0) {
        cpu_usage = 0.0;
    } else {
        cpu_usage = (1.0 - static_cast<double>(idle_delta) / total_delta) * 100.0;
    }

    last_times_ = current_times;

    return true;
}
