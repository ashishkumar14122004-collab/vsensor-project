# VSensor – Virtual Sensor Device Driver & Monitoring System

A Linux kernel module that simulates a hardware sensor device, exposing it through
the standard character device interface, paired with a C++ userspace monitoring dashboard.

## Project Structure

```
vsensor_project/
├── kernel/             # Linux kernel module (driver)
├── userspace/          # C++ monitoring application
├── docs/               # Documentation per stage
│   ├── stage1/
│   ├── stage2/
│   ├── stage3/
│   ├── stage4/
│   ├── stage5/
│   └── stage6/
├── tests/              # Unit and integration tests
├── scripts/            # Helper scripts (load/unload driver, etc.)
├── diagrams/           # UML and architecture diagrams (PlantUML source)
└── README.md
```

## Quick Start

```bash
# Build and load the kernel module
cd kernel && make
sudo insmod vsensor.ko

# Build and run the userspace app
cd userspace && cmake -B build && cmake --build build
./build/vsensor_monitor
```

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
