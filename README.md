# VSensor – Virtual Sensor Device Driver & Monitoring System

A Linux kernel module that simulates a hardware sensor device, exposing it through
the standard character device interface, paired with a C++ userspace monitoring dashboard.

> **Two modes available:**
> - **Real Mode** — requires the vsensor kernel module loaded on a Linux machine
> - **Demo Mode** — runs anywhere (GitHub Codespaces, WSL2, Docker) with no kernel module

## Project Structure

```
vsensor_project/
├── kernel/             # Linux kernel module (driver)
├── userspace/          # C++ monitoring application
│   ├── include/
│   │   ├── DemoReader.hpp      ← Demo Mode: reads /proc directly
│   │   └── ...                 ← Real Mode: all existing headers untouched
│   └── src/
│       ├── main.cpp            ← Entry point (branches to demo or real)
│       └── demo_main.cpp       ← Demo Mode display loop
├── docs/               # Documentation per stage
├── tests/
│   ├── test_demo_reader.cpp    ← Demo Mode unit tests
│   └── ...                     ← Existing unit tests untouched
├── scripts/            # Helper scripts (load/unload driver, etc.)
├── diagrams/           # UML and architecture diagrams (PlantUML source)
└── README.md
```

---

## Demo Mode (GitHub Codespaces / WSL2 / Docker — no kernel module needed)

Demo Mode reads **real system metrics** directly from `/proc` without loading any kernel module.

### Build

```bash
cd userspace
cmake -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build
```

### Run

```bash
./build/vsensor_monitor --demo
```

### With custom interval

```bash
./build/vsensor_monitor --demo --interval 2000
```

### Expected output

```
==========================================
   LINUX SYSTEM RESOURCE MONITOR
==========================================
   Mode: DEMO MODE  (sample #1)
   DEMO MODE - Kernel driver is not loaded
==========================================

  CPU Usage        : 12.3 %   [####--------------------------]
  Memory Total     : 7823 MB
  Memory Used      : 3241 MB
  Memory Available : 4582 MB
  Processes        : 147
  Uptime           : 2h 14m 33s

------------------------------------------
  Driver : DEMO / NOT LOADED
  Device : /dev/vsensor  (unavailable)
  Source : /proc/stat, /proc/meminfo,
           /proc/uptime, /proc/[PID]
------------------------------------------
  Press Ctrl+C to exit
```

### Why Demo Mode exists

The vsensor kernel module requires:
- A real Linux kernel (not available natively on Windows)
- Root privileges to load (`insmod`)
- Matching kernel headers installed

GitHub Codespaces and similar cloud environments run containers where
loading custom kernel modules is not permitted. Demo Mode lets you build,
run, and demonstrate the project in any of these environments.

**Demo Mode clearly states the kernel driver is NOT loaded.**
It does NOT simulate fake data — it reads real CPU, memory, and uptime
from the Linux `/proc` virtual filesystem.

---

## Real Linux Mode (Ubuntu VM / bare metal / WSL2 with kernel support)

### Build and load the kernel module

```bash
# Install dependencies
sudo apt update
sudo apt install -y build-essential linux-headers-$(uname -r) \
                    cmake libncurses-dev libgtest-dev git

# Build and load driver
cd kernel && make
sudo insmod vsensor.ko
sudo chmod 666 /dev/vsensor
dmesg | tail -5   # verify: [vsensor] registered at major=...
```

### Test the device directly

```bash
cat /dev/vsensor
# {"temp":37.4,"load":62.1,"ts":1728043200000}
```

### Build and run the monitor

```bash
cd userspace
cmake -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build
./build/vsensor_monitor --interval 500 --threshold 70.0
```

### Unload when done

```bash
sudo rmmod vsensor
```

---

## Run all unit tests (works in Demo environments too)

```bash
cd userspace
cmake -B build -DBUILD_TESTS=ON
cmake --build build
cd build && ctest --output-on-failure
```

Tests included:
| Suite | Tests | Requires kernel? |
|-------|-------|-----------------|
| CircularBufferTest | 9 | No |
| SensorParserTest | 6 | No |
| StatsEngineTest | 3 | No |
| AlertManagerTest | 4 | No |
| DemoReaderTest | 7 | No (reads /proc) |

---

## Stages
| Stage | Title | Status |
|-------|-------|--------|
| 1 | Project Introduction | ✅ Done |
| 2 | Requirements & Development Plan | ✅ Done |
| 3 | System Design & Architecture | ✅ Done |
| 4 | Initial Implementation & Prototype | ✅ Done |
| 5 | Testing, Integration & Improvement | ✅ Done |
| 6 | Final Implementation & Presentation | ✅ Done |

## Author
Individual Project – Linux, Device Drivers, System Programming & C++
