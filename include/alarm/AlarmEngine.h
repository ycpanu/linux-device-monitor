#pragma once

#include "collector/SystemMetrics.h"

#include <string>
#include <unordered_map>
#include <vector>

/*
 * AlarmEvent 表示一次告警事件。
 *
 * alarm_type：
 *   CPU_HIGH
 *   MEMORY_HIGH
 *   DISK_HIGH
 *
 * level：
 *   WARNING
 *   CRITICAL
 *
 * recovered：
 *   false 表示异常告警
 *   true  表示恢复事件
 */
struct AlarmEvent {
    std::string alarm_type;
    std::string level;
    std::string message;

    double current_value = 0.0;
    double threshold = 0.0;

    bool recovered = false;

    //告警发生时间
    long timestamp = 0;
};

/*
 * AlarmRule 保存告警规则。
 *
 * 这些值来自 config/config.json。
 */
struct AlarmRule {
    int continuous_count = 3;

    double cpu_warning_threshold = 70.0;
    double cpu_critical_threshold = 90.0;

    double memory_warning_threshold = 70.0;
    double memory_critical_threshold = 90.0;

    double disk_warning_threshold = 80.0;
    double disk_critical_threshold = 95.0;
};

/*
 * AlarmEngine 负责判断是否产生告警。
 *
 * 主要功能：
 * 1. 判断指标是否超过阈值
 * 2. 统计连续异常次数
 * 3. 生成 WARNING / CRITICAL 告警
 * 4. 指标恢复正常后生成恢复事件
 */
class AlarmEngine {
public:
    explicit AlarmEngine(const AlarmRule& rule);

    std::vector<AlarmEvent> checkMetrics(const SystemMetrics& metrics);

private:
    void checkValue(const std::string& alarm_type,
                    const std::string& display_name,
                    double current_value,
                    double warning_threshold,
                    double critical_threshold,
                    std::vector<AlarmEvent>& events);

    std::string formatDouble(double value) const;

private:
    AlarmRule rule_;

    /*
     * 记录每种告警连续异常的次数。
     * 例如：
     * CPU_HIGH -> 2
     * MEMORY_HIGH -> 1
     */
    std::unordered_map<std::string, int> abnormal_count_;

    /*
     * 记录某个告警当前是否已经处于报警状态。
     * 这样可以避免每 5 秒重复报警。
     */
    std::unordered_map<std::string, bool> alarming_;

    /*
     * 记录当前告警等级。
     * 用于从 WARNING 升级到 CRITICAL。
     */
    std::unordered_map<std::string, std::string> current_level_;
};
