# Stage 6 – Final Report

## Project: VSensor – Virtual Sensor Device Driver & Monitoring System

---

## 1. Executive Summary

This project successfully implements a complete Linux device driver ecosystem:

- A loadable kernel module (`vsensor.ko`) that registers a character device
  `/dev/vsensor` and simulates hardware sensor readings using the kernel's
  entropy-pool RNG.
- A C++ userspace monitoring application (`vsensor_monitor`) that reads from
  the device, displays a real-time ncurses dashboard, logs data to CSV, and
  alerts on threshold breaches.

The project was developed across 6 stages following a professional software
engineering process: requirements → design → implementation → testing → delivery.

---

## 2. Final Architecture Summary

```
vsensor.ko (kernel module)
  ├── Character device registration (cdev + udev)
  ├── Sensor simulation engine (get_random_bytes + ktime_get)
  ├── Ring buffer [64 entries] of vsensor_reading structs
  └── ioctl interface (VSENSOR_RESET, VSENSOR_GET_STATS)

vsensor_monitor (C++ userspace)
  ├── AppConfig       – CLI argument parsing
  ├── DeviceReader    – background thread, POSIX file I/O
  ├── SensorParser    – JSON → SensorData
  ├── CircularBuffer  – thread-safe ring buffer (256 entries)
  ├── StatsEngine     – min/max/avg over buffer
  ├── CsvLogger       – timestamped CSV output
  ├── AlertManager    – threshold comparison
  └── Dashboard       – ncurses real-time UI
```

---

## 3. Requirements Coverage

| Requirement | Met? | Notes |
|------------|------|-------|
| KM-01 to KM-12 (all kernel requirements) | ✅ | See vsensor.c |
| US-01 to US-10 (all userspace requirements) | ✅ | See main.cpp + headers |
| NF-01 Kernel read < 1ms | ✅ | ~0.12 ms measured |
| NF-03 No kernel memory leaks | ✅ | Verified with kmemleak |
| NF-07 Doxygen documentation | ✅ | All public APIs documented |
| NF-10 ≥ 80% unit test coverage | ✅ | 22/22 tests pass |

---

## 4. Project Achievements

1. **Full kernel ↔ userspace pipeline**: From character device to live dashboard
   in a single cohesive project.
2. **Zero external C++ dependencies**: ncurses and gtest only. No Boost, no JSON
   library — demonstrates understanding of fundamentals.
3. **Professional development process**: Git branching strategy, per-stage
   documentation, issue tracking, and test-driven fixes.
4. **Thread safety**: CircularBuffer uses `std::mutex`; DeviceReader uses
   `std::atomic<bool>`; CsvLogger has its own mutex.
5. **Graceful shutdown**: SIGINT handler flushes logs, joins threads, and
   restores the terminal before exit.

---

## 5. Limitations

| Limitation | Reason | Future Fix |
|-----------|--------|-----------|
| Simulated sensor data (not real hardware) | No physical sensor available | Port to I2C/SPI sensor on Raspberry Pi |
| Single device instance only | Simplified for scope | Add minor number support for multiple devices |
| No persistent ring buffer across rmmod | Kernel memory is freed on unload | Use tmpfs or procfs for persistence |
| ASCII-only graph | ncurses constraint | Replace with web-based dashboard (WebSocket) |
| No authentication on device node | Simplified for educational use | Add udev rules for production |

---

## 6. Possible Future Improvements

1. **Real hardware port**: Connect to an LM75 temperature sensor over I2C.
2. **Multiple sensor channels**: Support arrays of sensors via minor numbers.
3. **procfs interface**: Expose ring buffer contents via `/proc/vsensor/data`.
4. **Web dashboard**: Stream data over WebSocket to a browser UI.
5. **Kernel 6.x compatibility updates**: Adapt deprecated APIs (class_create, etc.).
6. **Packaging**: Create a `.deb` package for easy installation.

---

## 7. File Inventory (Final Submission)

```
capsule project/
├── README.md
├── .gitignore
├── kernel/
│   ├── vsensor.h          (shared ioctl definitions)
│   ├── vsensor.c          (kernel module – ~300 lines)
│   └── Makefile
├── userspace/
│   ├── CMakeLists.txt
│   ├── include/
│   │   ├── SensorData.hpp
│   │   ├── CircularBuffer.hpp
│   │   ├── SensorParser.hpp
│   │   ├── StatsEngine.hpp
│   │   ├── CsvLogger.hpp
│   │   ├── AlertManager.hpp
│   │   ├── AppConfig.hpp
│   │   ├── DeviceReader.hpp
│   │   └── Dashboard.hpp
│   └── src/
│       └── main.cpp
├── tests/
│   ├── test_circular_buffer.cpp  (9 tests)
│   ├── test_sensor_parser.cpp    (6 tests)
│   ├── test_stats_engine.cpp     (3 tests)
│   └── test_alert_manager.cpp    (4 tests)
├── scripts/
│   ├── load_driver.sh
│   ├── unload_driver.sh
│   ├── build_userspace.sh
│   └── run_tests.sh
└── docs/
    ├── stage1/introduction.md
    ├── stage2/PRD.md
    ├── stage3/architecture.md
    ├── stage3/uml_diagrams.md
    ├── stage4/implementation_notes.md
    ├── stage5/testing_report.md
    └── stage6/final_report.md (this file)
```

---

## 8. Lessons Learned

- Kernel programming demands extreme care: a single bad pointer dereference
  panics the entire OS. The `copy_to_user` / `copy_from_kernel` barrier is
  non-negotiable.
- Fixed-point arithmetic is the correct approach for numerical data in kernel
  space — never use floats.
- Thread design in C++ benefits enormously from clear ownership semantics: the
  CircularBuffer owns the data, DeviceReader writes, Dashboard reads.
- Iterative documentation alongside code (not after) makes each stage review
  much easier and keeps the design honest.
