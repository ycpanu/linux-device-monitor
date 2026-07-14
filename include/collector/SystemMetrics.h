#pragma once

#include <string>

/*
SystemMetrics 用来保存第一次采集得到的系统状态。
后续阶段可以继续扩展。
网络速率、温度、进程状态、服务状态等。
*/

struct SystemMetrics
{
    double cpu_usage = 0.0;       // CPU 使用率，单位：百分比，例如 35.5 表示 35.5%
    double memory_usage = 0.0;    // 内存使用率，单位：百分比
    double disk_usage = 0.0;      // 磁盘使用率，单位：百分比

    std::string disk_path = "/";  // 被监控的磁盘挂载点，默认监控根目录 /
    long timestamp = 0;
};


