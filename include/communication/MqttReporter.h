#pragma once

#include "alarm/AlarmEngine.h"
#include "collector/SystemMetrics.h"

#include <mosquitto.h>
#include <string>

/*
 * MqttConfig 保存 MQTT 连接配置。
 *
 * 这些配置来自 config/config.json。
 */
struct MqttConfig {
    bool enabled = false;
    std::string host = "localhost";
    int port = 1883;
    std::string client_id = "linux_gateway_001_agent";
    std::string topic_prefix = "device";
    int keepalive = 60;
};

/*
 * MqttReporter 负责通过 MQTT 上报数据。
 *
 * 本阶段支持三类上报：
 * 1. 设备状态：device/{device_id}/status
 * 2. 告警事件：device/{device_id}/alarm
 * 3. 心跳消息：device/{device_id}/heartbeat
 */
class MqttReporter {
public:
    MqttReporter();
    ~MqttReporter();

    bool init(const MqttConfig& config);

    bool reportMetrics(const std::string& device_id,
                       const SystemMetrics& metrics);

    bool reportAlarm(const std::string& device_id,
                     const AlarmEvent& alarm);

    bool reportHeartbeat(const std::string& device_id);

private:
    bool ensureConnected();
    bool publishJson(const std::string& topic, const std::string& payload);
    std::string buildTopic(const std::string& device_id,
                           const std::string& suffix) const;

private:
    MqttConfig config_;

    mosquitto* mosq_ = nullptr;

    bool initialized_ = false;
    bool connected_ = false;
    bool loop_started_ = false;
};
