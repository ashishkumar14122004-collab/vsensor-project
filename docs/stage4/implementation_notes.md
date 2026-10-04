# Stage 4 – Initial Implementation & Prototype

## 1. What Was Built

| Component | File(s) | Status |
|-----------|---------|--------|
| Kernel module | `kernel/vsensor.c`, `kernel/vsensor.h` | ✅ Complete |
| Kernel Makefile | `kernel/Makefile` | ✅ Complete |
| SensorData struct | `userspace/include/SensorData.hpp` | ✅ Complete |
| CircularBuffer<T> | `userspace/include/CircularBuffer.hpp` | ✅ Complete |
| SensorParser | `userspace/include/SensorParser.hpp` | ✅ Complete |
| StatsEngine | `userspace/include/StatsEngine.hpp` | ✅ Complete |
| CsvLogger | `userspace/include/CsvLogger.hpp` | ✅ Complete |
| AlertManager | `userspace/include/AlertManager.hpp` | ✅ Complete |
| AppConfig | `userspace/include/AppConfig.hpp` | ✅ Complete |
| DeviceReader | `userspace/include/DeviceReader.hpp` | ✅ Complete |
| Dashboard (ncurses) | `userspace/include/Dashboard.hpp` | ✅ Complete |
| main.cpp | `userspace/src/main.cpp` | ✅ Complete |
| CMakeLists.txt | `userspace/CMakeLists.txt` | ✅ Complete |
| Unit tests | `tests/test_*.cpp` | ✅ Complete |
| Helper scripts | `scripts/*.sh` | ✅ Complete |

---

## 2. How to Build and Run (on Linux)

### Step 1 – Install dependencies
```bash
sudo apt update
sudo apt install -y build-essential linux-headers-$(uname -r) \
                    cmake libncurses-dev libgtest-dev git doxygen
```

### Step 2 – Load the kernel module
```bash
cd kernel
make
sudo insmod vsensor.ko
sudo chmod 666 /dev/vsensor   # allow non-root reads
dmesg | tail -5               # verify: "[vsensor] registered at major=..."
```

### Step 3 – Quick device test
```bash
cat /dev/vsensor
# Expected: {"temp":37.4,"load":62.1,"ts":1728043200000}
```

### Step 4 – Build the userspace app
```bash
cd userspace
cmake -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build
```

### Step 5 – Run the monitor
```bash
./build/vsensor_monitor --interval 500 --threshold 70.0
```

### Step 6 – Unload driver when done
```bash
sudo rmmod vsensor
```

---

## 3. Key Design Decisions

### 3.1 Fixed-point arithmetic in kernel
Floating-point operations in kernel space require saving FPU context, which is
expensive and error-prone. Instead, all sensor values are stored as `int × 10`
(e.g., 37.4°C → `374`). Division by 10 happens in userspace.

### 3.2 JSON protocol
JSON was chosen for the read() response because:
- Human-readable (`cat /dev/vsensor` works for debugging)
- Trivially parseable without dependencies
- Extensible (new fields don't break old parsers)

### 3.3 Header-only userspace modules
Most C++ components are header-only to simplify the CMake setup and keep
compilation units minimal during prototyping.

### 3.4 Thread design
The producer-consumer split (DeviceReader thread → CircularBuffer → Dashboard
main thread) avoids blocking the UI and decouples I/O latency from rendering.

---

## 4. Issues Encountered & Solutions

| Issue | Root Cause | Solution |
|-------|------------|---------|
| `copy_to_user` returning -EFAULT | Buffer size mismatch between kernel/userspace | Added explicit size check before copy |
| `class_create` deprecated in kernel 6.4+ | API change: new signature requires no module ptr | Added `#ifdef` guard; kernel 5.x API used for compatibility |
| ncurses colors not showing in some terminals | `TERM` env var not set | Added `has_colors()` guard before color initialization |
| Device read returns same value twice | Missing `lseek(fd, 0, SEEK_SET)` after read | Added seek in DeviceReader loop |

---

## 5. Git Commit Log (this stage)

```
feat: add kernel module vsensor.c with device registration
feat: add ring buffer and sensor simulation engine
feat: add ioctl interface (VSENSOR_RESET, VSENSOR_GET_STATS)
feat: add SensorData, CircularBuffer, SensorParser headers
feat: add StatsEngine, CsvLogger, AlertManager
feat: add DeviceReader background thread
feat: add ncurses Dashboard with ASCII graph
feat: add main.cpp entry point and CMakeLists.txt
chore: add .gitignore and helper scripts
docs: add stage 4 implementation notes
```

---

## 6. Roadmap for Stage 5

- [ ] Run full unit test suite with coverage report
- [ ] Test driver load/unload under concurrent reads
- [ ] Test alert thresholds trigger correctly
- [ ] Fix any issues found during testing
- [ ] Add ioctl integration test (VSENSOR_GET_STATS)
- [ ] Performance profiling: verify read latency < 1 ms
- [ ] Code review: kernel coding style compliance
