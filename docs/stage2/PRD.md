# Stage 2 – Project Requirements Document (PRD)

## 1. Overview

| Field | Value |
|-------|-------|
| Project | VSensor – Virtual Sensor Device Driver & Monitoring System |
| Version | 1.0 |
| Date | 2026-10-04 |
| Status | Approved |

---

## 2. Functional Requirements

### 2.1 Kernel Module (vsensor.ko)

| ID | Requirement |
|----|------------|
| KM-01 | The module SHALL register a character device with major number allocated dynamically. |
| KM-02 | The module SHALL create the device node `/dev/vsensor` automatically via udev. |
| KM-03 | The device SHALL support `open`, `read`, `write`, `ioctl`, and `release` file operations. |
| KM-04 | A `read` call SHALL return a JSON-formatted string containing `temperature` (float, °C) and `cpu_load` (float, %) fields. |
| KM-05 | Sensor values SHALL be simulated using a pseudo-random algorithm seeded by `ktime_get()`. |
| KM-06 | Temperature SHALL be bounded between 20.0 and 85.0 °C. |
| KM-07 | CPU load SHALL be bounded between 0.0 and 100.0 %. |
| KM-08 | The module SHALL maintain an in-kernel ring buffer of the last 64 readings. |
| KM-09 | An `ioctl` command `VSENSOR_RESET` SHALL clear the ring buffer and reset statistics. |
| KM-10 | An `ioctl` command `VSENSOR_GET_STATS` SHALL return min, max, and average values. |
| KM-11 | The module SHALL print `[vsensor]` prefixed messages to `dmesg` on load, unload, and error. |
| KM-12 | The module SHALL be safely loadable and unloadable without kernel panic. |

### 2.2 Userspace Application (vsensor_monitor)

| ID | Requirement |
|----|------------|
| US-01 | The application SHALL open `/dev/vsensor` using standard POSIX `open()`. |
| US-02 | The application SHALL poll the device at a configurable interval (default: 1 second). |
| US-03 | The application SHALL parse the JSON response from the driver. |
| US-04 | The application SHALL display a real-time terminal dashboard using ncurses. |
| US-05 | The dashboard SHALL show: current temperature, current CPU load, min/max/avg statistics, and an ASCII history graph. |
| US-06 | The application SHALL write all readings to a timestamped CSV log file. |
| US-07 | The application SHALL support an alert threshold: if temperature > configurable limit, display a visual warning. |
| US-08 | The application SHALL handle `SIGINT` (Ctrl+C) gracefully, flushing logs and restoring terminal. |
| US-09 | The application SHALL support CLI arguments: `--interval`, `--logfile`, `--threshold`. |
| US-10 | The application SHALL use a separate reader thread and a main UI thread (producer-consumer pattern). |

---

## 3. Non-Functional Requirements

| ID | Category | Requirement |
|----|----------|------------|
| NF-01 | Performance | Kernel read latency SHALL be under 1 ms. |
| NF-02 | Performance | Userspace UI refresh SHALL not exceed 10 ms per frame. |
| NF-03 | Reliability | The driver SHALL not cause kernel oops or memory leaks (verified with kmemleak). |
| NF-04 | Reliability | The C++ app SHALL not crash on device unavailability; it SHALL retry with backoff. |
| NF-05 | Portability | The kernel module SHALL compile on Linux kernel 5.4 through 6.x. |
| NF-06 | Portability | The C++ app SHALL compile with GCC 11+ and Clang 14+. |
| NF-07 | Maintainability | All public APIs SHALL be documented with Doxygen comments. |
| NF-08 | Maintainability | Code SHALL follow Linux kernel coding style (kernel) and Google C++ Style Guide (userspace). |
| NF-09 | Security | The device node SHALL have permissions `0644`; write access requires root. |
| NF-10 | Testability | All userspace modules SHALL have unit tests with ≥ 80% line coverage. |

---

## 4. System Constraints

- Development OS: Ubuntu 22.04 LTS (kernel 5.15+)
- No physical hardware dependency
- Must run in a standard Linux VM or WSL2 with kernel module support
- External libraries: ncurses, gtest only

---

## 5. Modules & Features Breakdown

```
vsensor_project
├── [M1] Kernel Module
│   ├── Device registration & udev
│   ├── Sensor simulation engine
│   ├── Ring buffer
│   └── ioctl interface
├── [M2] JSON Protocol Layer
│   ├── Kernel-side formatter
│   └── Userspace parser (SensorData struct)
├── [M3] Reader Thread (C++)
│   ├── Device file I/O
│   ├── Circular buffer
│   └── Producer-consumer queue
├── [M4] Dashboard UI (ncurses)
│   ├── Layout manager
│   ├── ASCII graph renderer
│   └── Alert system
├── [M5] Logger
│   ├── CSV writer
│   └── File rotation
└── [M6] CLI & Configuration
    ├── Argument parser
    └── Config struct
```

---

## 6. Deliverables

| # | Deliverable | Stage |
|---|------------|-------|
| D1 | This PRD | 2 |
| D2 | System architecture document + diagrams | 3 |
| D3 | UML diagrams (Class, Sequence, State Machine) | 3 |
| D4 | Working kernel module prototype | 4 |
| D5 | Working C++ app prototype (basic read + log) | 4 |
| D6 | Full dashboard UI | 5 |
| D7 | Test suite + test report | 5 |
| D8 | Final source code + documentation | 6 |
| D9 | Git repository with full history | 6 |
| D10 | Final project report + presentation | 6 |

---

## 7. Development Plan & Timeline

| Week | Stage | Tasks |
|------|-------|-------|
| 1 | Stage 1 | Project introduction, define idea, scope |
| 2 | Stage 2 | PRD, requirements, development plan |
| 3 | Stage 3 | System architecture, component design |
| 4 | Stage 3 | UML diagrams, dev environment setup, Git init |
| 5 | Stage 4 | Kernel module skeleton + device registration |
| 6 | Stage 4 | Sensor simulation + ring buffer + ioctl |
| 7 | Stage 4 | C++ app skeleton + device reader + CSV logger |
| 8 | Stage 5 | Full ncurses dashboard, alert system |
| 9 | Stage 5 | Unit tests, integration tests, bug fixes |
| 10 | Stage 6 | Polish, final documentation, presentation prep |

---

## 8. Risk Register

| Risk | Probability | Impact | Mitigation |
|------|------------|--------|------------|
| Kernel module causes system crash | Medium | High | Test in VM snapshot; use kmemleak |
| WSL2 doesn't support custom kernel modules | Medium | High | Use full Ubuntu VM (VirtualBox/VMware) |
| ncurses complexity delays UI | Low | Medium | Start with plain stdout, migrate to ncurses |
| Scope creep | Low | Medium | Strict adherence to this PRD |
