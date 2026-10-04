#!/bin/bash
# unload_driver.sh – Unload the vsensor kernel module.
# Run as: sudo bash scripts/unload_driver.sh

set -e

echo "[*] Unloading vsensor module..."
sudo rmmod vsensor

echo "[+] Driver unloaded. Check dmesg for details:"
dmesg | tail -3
