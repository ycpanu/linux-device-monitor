#!/bin/bash

set -e

echo "[INFO] Stopping service..."
systemctl stop device-monitor || true

echo "[INFO] Disabling service..."
systemctl disable device-monitor || true

echo "[INFO] Removing files..."
rm -f /etc/systemd/system/device-monitor.service
rm -f /usr/local/bin/device-monitor
rm -rf /etc/device-monitor

echo "[INFO] Reloading systemd..."
systemctl daemon-reload

echo "[INFO] Uninstall completed."
echo ""
echo "Data directory is kept:"
echo "  /var/lib/device-monitor"
echo ""
echo "Log directory is kept:"
echo "  /var/log/device-monitor"
