#!/bin/bash
# build_userspace.sh – Configure and build the userspace application.

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
US_DIR="$SCRIPT_DIR/../userspace"

echo "[*] Configuring with CMake..."
cmake -S "$US_DIR" -B "$US_DIR/build" -DCMAKE_BUILD_TYPE=Debug

echo "[*] Building..."
cmake --build "$US_DIR/build" --parallel

echo "[+] Build complete: $US_DIR/build/vsensor_monitor"
