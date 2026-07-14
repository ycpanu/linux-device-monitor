#pragma once

#include "alarm/AlarmEngine.h"
#include "collector/SystemMetrics.h"

#include <sqlite3.h>
#include <string>

/*
 * SQLiteStorage 负责本地数据持久化。
 *
 * 第一版只保存：
 * 1. device_metrics：设备状态
 * 2. alarm_events：告警事件
 *
 * 后续可以继续扩展：
 * process_status、service_status。
 */
class SQLiteStorage {
public:
    SQLiteStorage();
    ~SQLiteStorage();

    /*
     * 打开数据库并创建表。
     */
    bool init(const std::string& db_path);

    /*
     * 保存一次设备状态。
     */
    bool saveMetrics(const std::string& device_id, const SystemMetrics& metrics);

    /*
     * 保存一次告警事件。
     */
    bool saveAlarm(const std::string& device_id, const AlarmEvent& alarm);

private:
    bool executeSql(const std::string& sql);
    bool createTables();

private:
    sqlite3* db_ = nullptr;
};
