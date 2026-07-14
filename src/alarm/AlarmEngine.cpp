#include "alarm/AlarmEngine.h"

#include <iomanip>
#include <sstream>
#include <ctime>

AlarmEngine::AlarmEngine(const AlarmRule& rule)
    : rule_(rule) {
}

/*
 * 将 double 保留两位小数。
 */
std::string AlarmEngine::formatDouble(double value) const {
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(2) << value;
    return oss.str();
}

/*
 * 检查所有系统指标。
 */
std::vector<AlarmEvent> AlarmEngine::checkMetrics(const SystemMetrics& metrics) {
    std::vector<AlarmEvent> events;

    checkValue("CPU_HIGH",
               "CPU",
               metrics.cpu_usage,
               rule_.cpu_warning_threshold,
               rule_.cpu_critical_threshold,
               events);

    checkValue("MEMORY_HIGH",
               "Memory",
               metrics.memory_usage,
               rule_.memory_warning_threshold,
               rule_.memory_critical_threshold,
               events);

    checkValue("DISK_HIGH",
               "Disk",
               metrics.disk_usage,
               rule_.disk_warning_threshold,
               rule_.disk_critical_threshold,
               events);

    return events;
}

/*
 * 检查单个指标是否触发告警。
 *
 * current_value >= critical_threshold：
 *   CRITICAL
 *
 * current_value >= warning_threshold：
 *   WARNING
 *
 * current_value < warning_threshold：
 *   正常
 */
void AlarmEngine::checkValue(const std::string& alarm_type,
                             const std::string& display_name,
                             double current_value,
                             double warning_threshold,
                             double critical_threshold,
                             std::vector<AlarmEvent>& events) {
    std::string level;
    double threshold = 0.0;

    if (current_value >= critical_threshold) {
        level = "CRITICAL";
        threshold = critical_threshold;
    } else if (current_value >= warning_threshold) {
        level = "WARNING";
        threshold = warning_threshold;
    }

    /*
     * 情况一：当前指标异常。
     */
    if (!level.empty()) {
        abnormal_count_[alarm_type]++;

        /*
         * 如果已经处于告警状态，则不重复发送同等级告警。
         *
         * 但是如果原来是 WARNING，现在变成 CRITICAL，
         * 需要输出一次升级告警。
         */
        if (alarming_[alarm_type]) {
            if (current_level_[alarm_type] == "WARNING" && level == "CRITICAL") {
                AlarmEvent event;
		event.timestamp = std::time(nullptr);
                event.alarm_type = alarm_type;
                event.level = level;
                event.current_value = current_value;
                event.threshold = threshold;
                event.recovered = false;
                event.message =
                    display_name + " alarm upgraded to CRITICAL, current value="
                    + formatDouble(current_value) + "%, threshold="
                    + formatDouble(threshold) + "%";

                current_level_[alarm_type] = level;
                events.push_back(event);
            }

            return;
        }

        /*
         * 连续异常次数达到配置要求，才真正报警。
         */
        if (abnormal_count_[alarm_type] >= rule_.continuous_count) {
            AlarmEvent event;
	    event.timestamp = std::time(nullptr);
            event.alarm_type = alarm_type;
            event.level = level;
            event.current_value = current_value;
            event.threshold = threshold;
            event.recovered = false;
            event.message =
                display_name + " usage is too high, current value="
                + formatDouble(current_value) + "%, threshold="
                + formatDouble(threshold) + "%";

            alarming_[alarm_type] = true;
            current_level_[alarm_type] = level;

            events.push_back(event);
        }

        return;
    }

    /*
     * 情况二：当前指标正常。
     *
     * 如果之前处于告警状态，现在恢复正常，则产生恢复事件。
     */
    abnormal_count_[alarm_type] = 0;

    if (alarming_[alarm_type]) {
        AlarmEvent event;
	event.timestamp = std::time(nullptr);
        event.alarm_type = alarm_type;
        event.level = "INFO";
        event.current_value = current_value;
        event.threshold = warning_threshold;
        event.recovered = true;
        event.message =
            display_name + " usage recovered, current value="
            + formatDouble(current_value) + "%";

        alarming_[alarm_type] = false;
        current_level_[alarm_type].clear();

        events.push_back(event);
    }
}
