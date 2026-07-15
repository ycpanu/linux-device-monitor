#!/bin/bash

DB_PATH="${1:-./data/monitor.db}"

echo "[INFO] Database path: $DB_PATH"

if [ ! -f "$DB_PATH" ]; then
    echo "[ERROR] Database file not found."
    exit 1
fi

echo "[INFO] Tables:"
sqlite3 "$DB_PATH" ".tables"

echo ""
echo "[INFO] Latest device metrics:"
sqlite3 "$DB_PATH" "
SELECT id, device_id, cpu_usage, memory_usage, disk_usage, created_at
FROM device_metrics
ORDER BY id DESC
LIMIT 5;
"

echo ""
echo "[INFO] Latest alarm events:"
sqlite3 "$DB_PATH" "
SELECT id, device_id, alarm_type, level, recovered, created_at
FROM alarm_events
ORDER BY id DESC
LIMIT 5;
"
