cmake_minimum_required(VERSION 3.16)

project(linux_device_monitor LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 头文件搜索目录
include_directories(include)

# 查找 nlohmann_json 依赖
find_package(nlohmann_json 3.2.0 REQUIRED)

add_executable(device-monitor
    src/main.cpp

    src/common/Logger.cpp

    src/config/ConfigManager.cpp

    src/collector/CpuCollector.cpp
    src/collector/MemoryCollector.cpp
    src/collector/DiskCollector.cpp
)

# 链接 JSON 库
target_link_libraries(device-monitor
    PRIVATE
    nlohmann_json::nlohmann_json
)
