# Linux 设备状态监控与预警系统

## 1. 项目简介

Linux 设备状态监控与预警系统是一个基于 C++ 开发的 Linux 设备端监控 Agent。

程序运行在 Linux 主机或嵌入式 Linux 设备上，周期性采集 CPU、内存、磁盘等系统状态，支持阈值告警、本地 SQLite 存储、MQTT 远程上报，并可通过 systemd 作为后台服务运行。

## 2. 核心功能

- CPU 使用率采集
- 内存使用率采集
- 磁盘使用率采集
- JSON 配置文件读取
- 本地日志输出
- 阈值告警判断
- 连续触发机制
- 告警恢复机制
- SQLite 本地持久化
- MQTT 状态上报
- MQTT 告警上报
- MQTT 心跳上报
- systemd 后台服务部署
- 开机自启动
- 异常自动重启

## 3. 技术栈

- C++17
- Linux `/proc`
- Linux `statvfs`
- CMake
- SQLite
- MQTT / Mosquitto
- nlohmann-json
- systemd
- Git

## 4. 项目结构

```text
linux-device-monitor/
├── CMakeLists.txt
├── README.md
├── config/
│   ├── config.json
│   └── config.service.json
├── docs/
├── include/
│   ├── alarm/
│   ├── collector/
│   ├── common/
│   ├── communication/
│   ├── config/
│   └── storage/
├── scripts/
│   ├── build.sh
│   ├── run.sh
│   ├── install.sh
│   ├── uninstall.sh
│   ├── check_db.sh
│   └── mqtt_sub.sh
├── service/
│   └── device-monitor.service
├── src/
│   ├── alarm/
│   ├── collector/
│   ├── common/
│   ├── communication/
│   ├── config/
│   ├── storage/
│   └── main.cpp
└── build/
```

## 5.环境依赖

sudo apt update

sudo apt install -y \
  build-essential \
  cmake \
  g++ \
  git \
  sqlite3 \
  libsqlite3-dev \
  nlohmann-json3-dev \
  mosquitto \
  mosquitto-clients \
  libmosquitto-dev
