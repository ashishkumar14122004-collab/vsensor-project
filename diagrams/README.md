# Diagrams

All UML diagrams are authored in PlantUML syntax in `docs/stage3/uml_diagrams.md`.

## How to render them

### Option 1 – Online (quickest)
1. Open https://www.plantuml.com/plantuml/uml/
2. Paste any `@startuml … @enduml` block from `docs/stage3/uml_diagrams.md`
3. Export as PNG or SVG

### Option 2 – Local (requires Java)
```bash
# Install PlantUML
sudo apt install -y plantuml

# Render all diagrams from the docs file
plantuml docs/stage3/uml_diagrams.md -o ../../diagrams/
```

### Option 3 – VS Code
Install the **PlantUML** extension (jebbs.plantuml), then open
`docs/stage3/uml_diagrams.md` and press Alt+D to preview.

---

## Diagram Index

| File (after render) | Description |
|---|---|
| `VSensor_ClassDiagram.png` | All C++ classes and their relationships |
| `VSensor_SequenceDiagram.png` | Normal read cycle from user to kernel and back |
| `VSensor_StateMachine.png` | DeviceReader thread state transitions |
| `KernelModule_StateMachine.png` | Kernel module lifecycle (load → serve → unload) |

---

## ASCII Architecture Diagram

The system-level architecture is also available in plain ASCII in
`docs/stage3/architecture.md` — no tools required to view it.

```
USER SPACE
  CLI/Config → DeviceReader → SensorParser → CircularBuffer → Dashboard
                                          └─→ CsvLogger
── syscall boundary ──────────────────────────────────────────────────
KERNEL SPACE
  vsensor.ko: cdev registration → sensor_engine → ring buffer → ioctl
                                         ↕
                               /dev/vsensor
```
