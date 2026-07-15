#!/bin/bash

DEVICE_ID="${1:-linux_gateway_001}"
HOST="${2:-localhost}"

TOPIC="device/${DEVICE_ID}/#"

echo "[INFO] MQTT host: $HOST"
echo "[INFO] Subscribe topic: $TOPIC"

mosquitto_sub -h "$HOST" -t "$TOPIC" -v
