#pragma once

#include <cstdint>

/*
 * CpuTimes 保存 /proc/stat 中 CPU 的时间数据。
 *
 * 这些值不是百分比，而是 Linux 内核统计的时间片数量。
 */
struct CpuTimes {
    uint64_t user = 0;
    uint64_t nice = 0;
    uint64_t system = 0;
    uint64_t idle = 0;
    uint64_t iowait = 0;
    uint64_t irq = 0;
    uint64_t softirq = 0;
    uint64_t steal = 0;
};

/*
 * CpuCollector 负责采集 CPU 使用率。
 *
 * CPU 使用率不能只读一次 /proc/stat。
 * 正确方式是：
 * 第一次读取 CPU 时间
 * 等待一小段时间
 * 第二次读取 CPU 时间
 * 根据两次差值计算 CPU 使用率
 */
class CpuCollector {
public:
    CpuCollector();

    bool collect(double& cpu_usage);

private:
    bool readCpuTimes(CpuTimes& times);
    uint64_t getIdleTime(const CpuTimes& times);
    uint64_t getTotalTime(const CpuTimes& times);

private:
    CpuTimes last_times_;
    bool has_last_ = false;
};
