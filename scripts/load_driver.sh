#!/bin/bash
# load_driver.sh – Build and load the vsensor kernel module.
# Run as: sudo bash scripts/load_driver.sh

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
KERNEL_DIR="$SCRIPT_DIR/../kernel"

echo "[*] Building kernel module..."
make -C "$KERNEL_DIR"

echo "[*] Loading vsensor.ko..."
sudo insmod "$KERNEL_DIR/vsensor.ko"

echo "[*] Setting /dev/vsensor permissions..."
sudo chmod 666 /dev/vsensor

echo "[+] Driver loaded. Check dmesg for details:"
dmesg | tail -5
