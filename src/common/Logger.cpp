#include "common/Logger.h"

#include <ctime>
#include <iomanip>
#include <iostream>
#include <sstream>

/*
 * 获取当前时间字符串。
 * 输出格式类似：
 * 2026-07-10 15:30:21
 */
std::string Logger::currentTime() {
    std::time_t now = std::time(nullptr);

    std::tm local_time{};

    /*
     * localtime_r 是线程安全版本。
     * 它会把时间戳转换成本地时间。
     */
    localtime_r(&now, &local_time);

    std::ostringstream oss;
    oss << std::put_time(&local_time, "%Y-%m-%d %H:%M:%S");

    return oss.str();
}

/*
 * 统一日志输出格式：
 * [时间] [日志等级] [模块名] 日志内容
 */
void Logger::log(const std::string& level,
                 const std::string& module,
                 const std::string& message) {
    std::cout << "[" << currentTime() << "] "
              << "[" << level << "] "
              << "[" << module << "] "
              << message << std::endl;
}

void Logger::debug(const std::string& module, const std::string& message) {
    log("DEBUG", module, message);
}

void Logger::info(const std::string& module, const std::string& message) {
    log("INFO", module, message);
}

void Logger::warn(const std::string& module, const std::string& message) {
    log("WARN", module, message);
}

void Logger::error(const std::string& module, const std::string& message) {
    log("ERROR", module, message);
}
