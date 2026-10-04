#!/bin/bash
# run_tests.sh – Build and run all unit tests.

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
US_DIR="$SCRIPT_DIR/../userspace"

echo "[*] Building with tests enabled..."
cmake -S "$US_DIR" -B "$US_DIR/build" -DBUILD_TESTS=ON
cmake --build "$US_DIR/build" --parallel

echo "[*] Running unit tests..."
cd "$US_DIR/build"
ctest --output-on-failure

echo "[+] All tests passed."
