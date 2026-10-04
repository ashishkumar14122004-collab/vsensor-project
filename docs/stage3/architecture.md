# Stage 3 – System Design & Architecture

## 1. System Overview

The VSensor system is split across two privilege levels of the Linux OS:

```
┌─────────────────────────────────────────────────────┐
│                   USER SPACE                        │
│                                                     │
│  ┌──────────────┐    ┌─────────────┐   ┌────────┐  │
│  │ CLI / Config │───▶│ Reader      │──▶│ Parser │  │
│  └──────────────┘    │ Thread      │   └───┬────┘  │
│                      └─────────────┘       │       │
│                            │               ▼       │
│                      ┌─────▼──────┐  ┌─────────┐  │
│                      │ Circ.Buffer│  │ Logger  │  │
│                      └─────┬──────┘  └─────────┘  │
│                            │                       │
│                      ┌─────▼──────┐               │
│                      │  Dashboard │               │
│                      │  (ncurses) │               │
│                      └────────────┘               │
│                                                     │
├─────────────────── syscall boundary ────────────────┤
│                                                     │
│                  KERNEL SPACE                       │
│                                                     │
│  ┌──────────────────────────────────────────────┐  │
│  │            vsensor.ko (kernel module)        │  │
│  │                                              │  │
│  │  ┌─────────────┐   ┌────────────────────┐   │  │
│  │  │  File Ops   │   │  Sensor Simulator  │   │  │
│  │  │ open/read/  │◀──│  (ktime seed RNG)  │   │  │
│  │  │ write/ioctl │   └────────────────────┘   │  │
│  │  └──────┬──────┘                            │  │
│  │         │        ┌──────────────────────┐   │  │
│  │         └───────▶│   Ring Buffer (64)   │   │  │
│  │                  └──────────────────────┘   │  │
│  │                                              │  │
│  │  ┌──────────────────────────────────────┐   │  │
│  │  │  Device Registration (cdev + udev)   │   │  │
│  │  └──────────────────────────────────────┘   │  │
│  └──────────────────────────────────────────────┘  │
│                      /dev/vsensor                   │
└─────────────────────────────────────────────────────┘
```

---

## 2. Major Components

### 2.1 Kernel Module (`vsensor.ko`)

| Sub-component | Responsibility |
|---------------|---------------|
| `device_init` | Allocate major number, create cdev, register with kernel |
| `device_exit` | Unregister cdev, destroy device node, free resources |
| `vsensor_open` | Validate access, initialize per-file state |
| `vsensor_read` | Generate sensor reading, format as JSON, copy to userspace |
| `vsensor_write` | Accept threshold config from userspace |
| `vsensor_ioctl` | Handle VSENSOR_RESET and VSENSOR_GET_STATS commands |
| `vsensor_release` | Clean up per-file state |
| `sensor_engine` | Pseudo-random value generation within bounds |
| `ring_buffer` | Fixed-size circular buffer of `vsensor_reading` structs |

### 2.2 Userspace Application

| Component | Class/Module | Responsibility |
|-----------|-------------|---------------|
| Entry point | `main.cpp` | Parse CLI args, spin up threads, run event loop |
| Device reader | `DeviceReader` | Open device, poll with configurable interval |
| Data parser | `SensorParser` | Parse JSON string into `SensorData` struct |
| Data buffer | `CircularBuffer<T>` | Thread-safe ring buffer for history |
| Statistics | `StatsEngine` | Compute min, max, average, rolling average |
| Logger | `CsvLogger` | Write timestamped CSV entries to file |
| Dashboard | `Dashboard` | ncurses-based real-time display |
| Alert system | `AlertManager` | Compare values against thresholds, trigger warnings |
| Config | `AppConfig` | Hold CLI-parsed configuration values |

---

## 3. Data Structures

### 3.1 Kernel: `vsensor_reading` (C struct)
```c
struct vsensor_reading {
    s64       timestamp_ns;   /* ktime_get() nanoseconds */
    int       temperature_x10; /* temp * 10, e.g. 253 = 25.3 °C */
    int       cpu_load_x10;    /* load * 10, e.g. 672 = 67.2 % */
};
```

### 3.2 Kernel: `vsensor_stats` (ioctl response)
```c
struct vsensor_stats {
    int temp_min_x10;
    int temp_max_x10;
    int temp_avg_x10;
    int load_min_x10;
    int load_max_x10;
    int load_avg_x10;
    u32 sample_count;
};
```

### 3.3 Userspace: `SensorData` (C++ struct)
```cpp
struct SensorData {
    std::chrono::system_clock::time_point timestamp;
    float temperature;   // °C
    float cpu_load;      // %
};
```

### 3.4 Userspace: `CircularBuffer<T>`
```
Template circular buffer
- Fixed capacity (default 256 entries)
- Thread-safe via std::mutex
- Supports: push(), latest(), range(n), full statistics
```

---

## 4. Communication Protocol

The kernel module returns data in a simple JSON string on each `read()`:

```json
{"temp": 37.4, "load": 62.1, "ts": 1728043200000}
```

- `temp`: float, degrees Celsius
- `load`: float, percent
- `ts`: uint64, Unix timestamp milliseconds

The userspace parser uses a minimal hand-written parser (no external JSON library)
to avoid dependencies.

---

## 5. Threading Model

```
Main Thread
    │
    ├── spawn ──▶  Reader Thread
    │               │  reads /dev/vsensor every N ms
    │               │  pushes SensorData to lock-free queue
    │               │
    ├── spawn ──▶  Logger Thread
    │               │  drains queue, writes CSV
    │               │
    └── runs ──▶  UI Thread (main)
                    │  reads CircularBuffer
                    │  refreshes ncurses dashboard
```

Synchronization: `std::mutex` + `std::condition_variable` producer-consumer queue.

---

## 6. ioctl Interface

Defined in shared header `vsensor_ioctl.h` (used by both kernel and userspace):

```c
#define VSENSOR_MAGIC    'V'
#define VSENSOR_RESET    _IO(VSENSOR_MAGIC,  0)
#define VSENSOR_GET_STATS _IOR(VSENSOR_MAGIC, 1, struct vsensor_stats)
```

---

## 7. Git Branching Strategy

```
main
 └── develop
      ├── feature/kernel-module
      ├── feature/device-reader
      ├── feature/dashboard
      ├── feature/logger
      └── feature/tests
```

- `main`: stable, tagged releases only
- `develop`: integration branch
- `feature/*`: individual feature development
- Merge via pull request with commit message convention: `[type]: description`
  - Types: `feat`, `fix`, `docs`, `test`, `refactor`, `chore`

---

## 8. Development Environment Setup

```bash
# Install dependencies (Ubuntu 22.04)
sudo apt update
sudo apt install -y build-essential linux-headers-$(uname -r) \
                    cmake libncurses-dev libgtest-dev doxygen git

# Clone and initialize
git clone <repo_url> vsensor_project
cd vsensor_project
git checkout -b develop
git checkout -b feature/kernel-module
```
