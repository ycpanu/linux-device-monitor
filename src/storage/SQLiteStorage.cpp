#include "storage/SQLiteStorage.h"
#include "common/Logger.h"

SQLiteStorage::SQLiteStorage() = default;

SQLiteStorage::~SQLiteStorage() {
    /*
     * 程序退出时关闭数据库连接。
     */
    if (db_ != nullptr) {
        sqlite3_close(db_);
        db_ = nullptr;
    }
}

/*
 * 初始化数据库。
 *
 * sqlite3_open:
 * 1. 如果数据库文件不存在，会自动创建。
 * 2. 如果数据库文件存在，会直接打开。
 * 3. 如果目录不存在，会打开失败。
 */
bool SQLiteStorage::init(const std::string& db_path) {
    int ret = sqlite3_open(db_path.c_str(), &db_);

    if (ret != SQLITE_OK) {
        Logger::error("storage", "failed to open database: " + db_path);
        return false;
    }

    Logger::info("storage", "database opened: " + db_path);

    return createTables();
}

/*
 * 执行一条 SQL。
 *
 * sqlite3_exec 用于执行不需要返回结果的 SQL，
 * 例如 CREATE TABLE、INSERT、UPDATE、DELETE。
 */
bool SQLiteStorage::executeSql(const std::string& sql) {
    char* error_message = nullptr;

    int ret = sqlite3_exec(db_, sql.c_str(), nullptr, nullptr, &error_message);

    if (ret != SQLITE_OK) {
        std::string message = error_message ? error_message : "unknown error";
        Logger::error("storage", "execute sql failed: " + message);

        if (error_message != nullptr) {
            sqlite3_free(error_message);
        }

        return false;
    }

    return true;
}

/*
 * 创建数据库表。
 *
 * IF NOT EXISTS 表示：
 * 如果表不存在就创建；
 * 如果表已经存在，不会报错。
 */
bool SQLiteStorage::createTables() {
    const std::string create_metrics_table = R"(
        CREATE TABLE IF NOT EXISTS device_metrics (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            device_id TEXT NOT NULL,
            cpu_usage REAL,
            memory_usage REAL,
            disk_usage REAL,
            disk_path TEXT,
            created_at INTEGER NOT NULL
        );
    )";

    const std::string create_alarm_table = R"(
        CREATE TABLE IF NOT EXISTS alarm_events (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            device_id TEXT NOT NULL,
            alarm_type TEXT NOT NULL,
            level TEXT NOT NULL,
            message TEXT,
            current_value REAL,
            threshold_value REAL,
            recovered INTEGER DEFAULT 0,
            created_at INTEGER NOT NULL
        );
    )";

    if (!executeSql(create_metrics_table)) {
        return false;
    }

    if (!executeSql(create_alarm_table)) {
        return false;
    }

    Logger::info("storage", "database tables initialized");
    return true;
}

/*
 * 保存设备状态。
 *
 * 这里使用 sqlite3_prepare_v2 + sqlite3_bind_xxx，
 * 不直接拼接 SQL。
 *
 * 原因：
 * 1. 更安全
 * 2. 可以避免字符串里有特殊字符导致 SQL 出错
 * 3. 更接近企业项目写法
 */
bool SQLiteStorage::saveMetrics(const std::string& device_id,
                                const SystemMetrics& metrics) {
    const char* sql = R"(
        INSERT INTO device_metrics (
            device_id,
            cpu_usage,
            memory_usage,
            disk_usage,
            disk_path,
            created_at
        ) VALUES (?, ?, ?, ?, ?, ?);
    )";

    sqlite3_stmt* stmt = nullptr;

    if (sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        Logger::error("storage", "prepare saveMetrics sql failed");
        return false;
    }

    /*
     * SQLite 参数下标从 1 开始，不是从 0 开始。
     */
    sqlite3_bind_text(stmt, 1, device_id.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_double(stmt, 2, metrics.cpu_usage);
    sqlite3_bind_double(stmt, 3, metrics.memory_usage);
    sqlite3_bind_double(stmt, 4, metrics.disk_usage);
    sqlite3_bind_text(stmt, 5, metrics.disk_path.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int64(stmt, 6, metrics.timestamp);

    bool ok = true;

    if (sqlite3_step(stmt) != SQLITE_DONE) {
        Logger::error("storage", "insert device_metrics failed");
        ok = false;
    }

    /*
     * sqlite3_finalize 用来释放 stmt。
     * 只要 prepare 成功了，最后都应该 finalize。
     */
    sqlite3_finalize(stmt);

    return ok;
}

/*
 * 保存告警事件。
 */
bool SQLiteStorage::saveAlarm(const std::string& device_id,
                              const AlarmEvent& alarm) {
    const char* sql = R"(
        INSERT INTO alarm_events (
            device_id,
            alarm_type,
            level,
            message,
            current_value,
            threshold_value,
            recovered,
            created_at
        ) VALUES (?, ?, ?, ?, ?, ?, ?, ?);
    )";

    sqlite3_stmt* stmt = nullptr;

    if (sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        Logger::error("storage", "prepare saveAlarm sql failed");
        return false;
    }

    sqlite3_bind_text(stmt, 1, device_id.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, alarm.alarm_type.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, alarm.level.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 4, alarm.message.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_double(stmt, 5, alarm.current_value);
    sqlite3_bind_double(stmt, 6, alarm.threshold);
    sqlite3_bind_int(stmt, 7, alarm.recovered ? 1 : 0);
    sqlite3_bind_int64(stmt, 8, alarm.timestamp);

    bool ok = true;

    if (sqlite3_step(stmt) != SQLITE_DONE) {
        Logger::error("storage", "insert alarm_events failed");
        ok = false;
    }

    sqlite3_finalize(stmt);

    return ok;
}
