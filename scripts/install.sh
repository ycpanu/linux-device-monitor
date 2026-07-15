#!/bin/bash

set -e

PROJECT_ROOT="$(cd "$(dirname "$0")/.." && pwd)"

BINARY_PATH="$PROJECT_ROOT/build/device-monitor"
CONFIG_PATH="$PROJECT_ROOT/config/config.service.json"
SERVICE_PATH="$PROJECT_ROOT/service/device-monitor.service"

INSTALL_BIN="/usr/local/bin/device-monitor"
INSTALL_CONFIG_DIR="/etc/device-monitor"
INSTALL_CONFIG="$INSTALL_CONFIG_DIR/config.json"
INSTALL_DATA_DIR="/var/lib/device-monitor"
INSTALL_LOG_DIR="/var/log/device-monitor"
INSTALL_SERVICE="/etc/systemd/system/device-monitor.service"

echo "[INFO] Project root: $PROJECT_ROOT"

if [ ! -f "$BINARY_PATH" ]; then
    echo "[ERROR] Binary not found: $BINARY_PATH"
    echo "[INFO] Please run:"
    echo "       cd build && cmake .. && make"
    exit 1
fi

if [ ! -f "$CONFIG_PATH" ]; then
    echo "[ERROR] Config file not found: $CONFIG_PATH"
    exit 1
fi

if [ ! -f "$SERVICE_PATH" ]; then
    echo "[ERROR] Service file not found: $SERVICE_PATH"
    exit 1
fi

echo "[INFO] Creating directories..."
mkdir -p "$INSTALL_CONFIG_DIR"
mkdir -p "$INSTALL_DATA_DIR"
mkdir -p "$INSTALL_LOG_DIR"

echo "[INFO] Installing binary..."
cp "$BINARY_PATH" "$INSTALL_BIN"
chmod +x "$INSTALL_BIN"

echo "[INFO] Installing config..."
cp "$CONFIG_PATH" "$INSTALL_CONFIG"

echo "[INFO] Installing systemd service..."
cp "$SERVICE_PATH" "$INSTALL_SERVICE"

echo "[INFO] Reloading systemd..."
systemctl daemon-reload

echo "[INFO] Enabling service..."
systemctl enable device-monitor

echo "[INFO] Installation completed."
echo ""
echo "You can now run:"
echo "  sudo systemctl start device-monitor"
echo "  sudo systemctl status device-monitor"
echo "  sudo journalctl -u device-monitor -f"
