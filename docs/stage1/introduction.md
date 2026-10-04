# Stage 1 – Project Introduction

## 1. Project Title
**VSensor – Virtual Sensor Device Driver & Real-Time Monitoring System**

---

## 2. Project Objective

The objective of this project is to design and implement a complete Linux-based
system that demonstrates the full software stack from kernel space to userspace:

- Write a Linux **character device driver** (kernel module) that simulates a
  virtual hardware sensor producing temperature and CPU-load readings.
- Develop a **C++ userspace application** that opens the device file, reads sensor
  data continuously, and presents it in a real-time terminal dashboard.
- Apply professional software engineering practices: version control, documentation,
  unit testing, and incremental delivery across 6 stages.

---

## 3. Problem Statement

Embedded and systems engineers frequently need to:
1. Develop and validate device drivers **before** physical hardware is available.
2. Monitor hardware sensor data from userspace applications without relying on
   third-party frameworks.

There is no standard, educational, end-to-end example that connects a custom kernel
module to a structured C++ monitoring application. This project fills that gap by
building a self-contained virtual sensor ecosystem that mirrors a real production workflow.

---

## 4. Project Scope

### In Scope
| Item | Description |
|------|-------------|
| Kernel module | Character device driver (`/dev/vsensor`) written in C |
| Sensor simulation | Pseudo-random temperature (20–85 °C) and CPU load (0–100 %) |
| Device interface | Standard `open`, `read`, `write`, `ioctl`, `release` file operations |
| Userspace app | C++17 application reading data via the device file |
| Dashboard | Real-time terminal UI using ncurses |
| Logging | Timestamped CSV log file written to disk |
| Data structures | Ring buffer in kernel; circular buffer + queue in userspace |
| Build system | Makefile (kernel), CMake (userspace) |
| Testing | Unit tests for C++ modules; basic kernel module load/unload tests |
| Documentation | Per-stage Markdown docs + Doxygen API reference |
| Version control | Git with feature-branch workflow |

### Out of Scope
- Physical hardware integration
- GUI application (web or desktop)
- Cross-compilation for embedded targets (Raspberry Pi, etc.)
- Network-based sensor streaming

---

## 5. Expected Outcome

By the end of Stage 6 the project will deliver:

1. **`vsensor.ko`** – A loadable Linux kernel module exposing `/dev/vsensor`.
2. **`vsensor_monitor`** – A C++ binary that reads from the device and displays
   a live dashboard showing current readings, history graphs (ASCII), min/max
   statistics, and alert thresholds.
3. **CSV log file** – Persistent timestamped sensor log.
4. **Full documentation** – PRD, architecture diagrams, UML, Doxygen HTML,
   test reports, and a final project report.
5. **Git repository** – Full commit history demonstrating incremental progress.

---

## 6. Application / Real-World Relevance

| Domain | Relevance |
|--------|-----------|
| Embedded Systems | Mirrors the workflow of writing drivers for sensors (I2C, SPI) |
| Automotive | ECU data monitoring pipelines follow the same kernel ↔ userspace pattern |
| IoT | Device abstraction and data logging are core IoT primitives |
| Education | Demonstrates the complete Linux driver development lifecycle |

---

## 7. Technology Stack

| Layer | Technology |
|-------|-----------|
| Kernel module | C (GNU C99, Linux Kernel API 5.x+) |
| Userspace app | C++17 (GCC/Clang) |
| Terminal UI | ncurses |
| Build (kernel) | Kbuild / Makefile |
| Build (userspace) | CMake 3.16+ |
| Testing | Google Test (gtest) |
| Documentation | Doxygen, Markdown |
| Version control | Git |
| OS | Ubuntu 22.04 LTS (or any Linux 5.x kernel) |

---

## 8. Stage Roadmap

| Stage | Deliverable | Target |
|-------|------------|--------|
| 1 | Project Introduction (this document) | Week 1 |
| 2 | PRD + Development Plan | Week 2 |
| 3 | System Design, Architecture, UML | Week 3–4 |
| 4 | Initial Implementation & Prototype | Week 5–7 |
| 5 | Testing, Integration & Fixes | Week 8–9 |
| 6 | Final Delivery & Presentation | Week 10 |
