#include "communication/MqttReporter.h"
#include "common/Logger.h"

#include <ctime>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

MqttReporter::MqttReporter() = default;

MqttReporter::~MqttReporter() {
    /*
     * 程序退出时释放 MQTT 相关资源。
     */
    if (mosq_ != nullptr) {
        if (connected_) {
            mosquitto_disconnect(mosq_);
        }

        if (loop_started_) {
            mosquitto_loop_stop(mosq_, true);
        }

        mosquitto_destroy(mosq_);
        mosq_ = nullptr;
    }

    if (initialized_) {
        mosquitto_lib_cleanup();
    }
}

/*
 * 初始化 MQTT 客户端。
 *
 * 注意：
 * 这里不会因为 MQTT 连接失败就让主程序退出。
 * 因为设备监控程序应该具备一定容错能力：
 * MQTT 暂时不可用时，本地采集和 SQLite 存储仍然继续运行。
 */
bool MqttReporter::init(const MqttConfig& config) {
    config_ = config;

    if (!config_.enabled) {
        Logger::info("mqtt", "mqtt reporter disabled");
        return true;
    }

    mosquitto_lib_init();
    initialized_ = true;

    /*
     * mosquitto_new 参数说明：
     * 1. client_id：客户端 ID
     * 2. clean_session：是否清理会话，true 表示每次重新建立新会话
     * 3. obj：用户自定义指针，这里暂时不用
     */
    mosq_ = mosquitto_new(config_.client_id.c_str(), true, nullptr);

    if (mosq_ == nullptr) {
        Logger::error("mqtt", "failed to create mqtt client");
        return false;
    }

    Logger::info("mqtt", "mqtt client created");

    /*
     * 先尝试连接一次。
     * 如果连接失败，后续上报时会继续尝试重连。
     */
    ensureConnected();

    return true;
}

/*
 * 确保 MQTT 已连接。
 *
 * 如果当前未连接，就尝试连接 Broker。
 */
bool MqttReporter::ensureConnected() {
    if (!config_.enabled) {
        return false;
    }

    if (connected_) {
        return true;
    }

    if (mosq_ == nullptr) {
        return false;
    }

    int ret = mosquitto_connect(
        mosq_,
        config_.host.c_str(),
        config_.port,
        config_.keepalive
    );

    if (ret != MOSQ_ERR_SUCCESS) {
        Logger::warn("mqtt", std::string("connect failed: ") + mosquitto_strerror(ret));
        connected_ = false;
        return false;
    }

    /*
     * mosquitto_loop_start 会启动一个后台线程，
     * 负责处理 MQTT 网络收发。
     */
    if (!loop_started_) {
        ret = mosquitto_loop_start(mosq_);
        if (ret != MOSQ_ERR_SUCCESS) {
            Logger::warn("mqtt", std::string("loop start failed: ") + mosquitto_strerror(ret));
            connected_ = false;
            return false;
        }

        loop_started_ = true;
    }

    connected_ = true;
    Logger::info("mqtt", "connected to mqtt broker");

    return true;
}

/*
 * 拼接 MQTT Topic。
 *
 * 例如：
 * prefix = device
 * device_id = linux_gateway_001
 * suffix = status
 *
 * 最终 topic：
 * device/linux_gateway_001/status
 */
std::string MqttReporter::buildTopic(const std::string& device_id,
                                     const std::string& suffix) const {
    return config_.topic_prefix + "/" + device_id + "/" + suffix;
}

/*
 * 发布 JSON 字符串。
 */
bool MqttReporter::publishJson(const std::string& topic,
                               const std::string& payload) {
    if (!config_.enabled) {
        return true;
    }

    if (!ensureConnected()) {
        return false;
    }

    /*
     * mosquitto_publish 参数说明：
     * 1. mosq_：MQTT 客户端对象
     * 2. mid：消息 ID，这里不需要，所以传 nullptr
     * 3. topic：主题
     * 4. payloadlen：消息长度
     * 5. payload：消息内容
     * 6. qos：服务质量等级，1 表示至少送达一次
     * 7. retain：是否保留消息，false 表示不保留
     */
    int ret = mosquitto_publish(
        mosq_,
        nullptr,
        topic.c_str(),
        static_cast<int>(payload.size()),
        payload.c_str(),
        1,
        false
    );

    if (ret != MOSQ_ERR_SUCCESS) {
        Logger::warn("mqtt", std::string("publish failed: ") + mosquitto_strerror(ret));
        connected_ = false;
        return false;
    }

    Logger::info("mqtt", "published message to topic: " + topic);
    return true;
}

/*
 * 上报设备状态。
 */
bool MqttReporter::reportMetrics(const std::string& device_id,
                                 const SystemMetrics& metrics) {
    json j;

    j["device_id"] = device_id;
    j["timestamp"] = metrics.timestamp;
    j["cpu_usage"] = metrics.cpu_usage;
    j["memory_usage"] = metrics.memory_usage;
    j["disk_usage"] = metrics.disk_usage;
    j["disk_path"] = metrics.disk_path;

    std::string topic = buildTopic(device_id, "status");

    return publishJson(topic, j.dump());
}

/*
 * 上报告警事件。
 */
bool MqttReporter::reportAlarm(const std::string& device_id,
                               const AlarmEvent& alarm) {
    json j;

    j["device_id"] = device_id;
    j["timestamp"] = alarm.timestamp;
    j["alarm_type"] = alarm.alarm_type;
    j["level"] = alarm.level;
    j["message"] = alarm.message;
    j["current_value"] = alarm.current_value;
    j["threshold"] = alarm.threshold;
    j["recovered"] = alarm.recovered;

    std::string topic = buildTopic(device_id, "alarm");

    return publishJson(topic, j.dump());
}

/*
 * 上报心跳。
 *
 * 心跳用于告诉远程平台：
 * 这个设备端 Agent 还活着。
 */
bool MqttReporter::reportHeartbeat(const std::string& device_id) {
    json j;

    j["device_id"] = device_id;
    j["timestamp"] = std::time(nullptr);
    j["status"] = "online";

    std::string topic = buildTopic(device_id, "heartbeat");

    return publishJson(topic, j.dump());
}
