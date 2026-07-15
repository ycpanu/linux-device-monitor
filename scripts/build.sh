#!/bin/bash

set -e

PROJECT_ROOT="$(cd "$(dirname "$0")/.." && pwd)"

echo "[INFO] Project root: $PROJECT_ROOT"

cd "$PROJECT_ROOT"

if [ ! -d build ]; then
    echo "[INFO] Creating build directory..."
    mkdir build
fi

cd build

echo "[INFO] Running cmake..."
cmake ..

echo "[INFO] Building project..."
make -j$(nproc)

echo "[INFO] Build completed."
