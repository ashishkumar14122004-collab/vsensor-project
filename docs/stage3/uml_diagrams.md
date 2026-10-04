# Stage 3 – UML Diagrams

All diagrams are written in PlantUML syntax. Render them at https://www.plantuml.com/plantuml

---

## 1. Class Diagram

```plantuml
@startuml VSensor_ClassDiagram

skinparam classAttributeIconSize 0

class AppConfig {
  + interval_ms : int
  + logfile     : string
  + threshold   : float
  + device_path : string
  + parse(argc, argv) : bool
}

class SensorData {
  + timestamp   : time_point
  + temperature : float
  + cpu_load    : float
}

class SensorParser {
  + parse(raw : string) : SensorData
  - extractFloat(json, key) : float
  - extractLong(json, key) : long
}

class CircularBuffer<T> {
  - buffer   : vector<T>
  - head     : size_t
  - tail     : size_t
  - count    : size_t
  - capacity : size_t
  - mtx      : mutex
  + push(item : T) : void
  + latest() : T
  + range(n) : vector<T>
  + size()   : size_t
  + full()   : bool
}

class StatsEngine {
  - buffer : CircularBuffer<SensorData>&
  + minTemp()  : float
  + maxTemp()  : float
  + avgTemp()  : float
  + minLoad()  : float
  + maxLoad()  : float
  + avgLoad()  : float
}

class DeviceReader {
  - fd          : int
  - config      : AppConfig&
  - buffer      : CircularBuffer<SensorData>&
  - running     : atomic<bool>
  + start() : void
  + stop()  : void
  - run()   : void
  - readOnce() : string
}

class CsvLogger {
  - file    : ofstream
  - mtx     : mutex
  + open(path : string) : bool
  + log(data : SensorData) : void
  + close() : void
}

class AlertManager {
  - temp_threshold : float
  - load_threshold : float
  + checkTemp(val : float) : bool
  + checkLoad(val : float) : bool
  + setTempThreshold(t : float) : void
  + setLoadThreshold(t : float) : void
}

class Dashboard {
  - stats   : StatsEngine&
  - buffer  : CircularBuffer<SensorData>&
  - alerts  : AlertManager&
  - config  : AppConfig&
  + init()    : void
  + render()  : void
  + cleanup() : void
  - drawHeader()      : void
  - drawCurrentValues() : void
  - drawGraph()       : void
  - drawStats()       : void
  - drawAlerts()      : void
}

AppConfig       <-- DeviceReader   : uses
AppConfig       <-- Dashboard      : uses
SensorData      <-- CircularBuffer : stores
SensorData      <-- SensorParser   : produces
SensorData      <-- CsvLogger      : logs
CircularBuffer  <-- DeviceReader   : writes
CircularBuffer  <-- StatsEngine    : reads
CircularBuffer  <-- Dashboard      : reads
StatsEngine     <-- Dashboard      : uses
AlertManager    <-- Dashboard      : uses
SensorParser    <-- DeviceReader   : uses
CsvLogger       <-- DeviceReader   : uses

@enduml
```

---

## 2. Sequence Diagram – Normal Read Cycle

```plantuml
@startuml VSensor_SequenceDiagram

actor       User
participant "main()"          as Main
participant "DeviceReader"     as DR
participant "/dev/vsensor"     as DEV
participant "vsensor.ko"       as KM
participant "SensorParser"     as SP
participant "CircularBuffer"   as CB
participant "CsvLogger"        as LOG
participant "Dashboard"        as DASH

User  -> Main  : launch vsensor_monitor
Main  -> DR    : start()
Main  -> DASH  : init()

loop every interval_ms
  DR   -> DEV  : read()
  DEV  -> KM   : vsensor_read() [kernel]
  KM   -> KM   : sensor_engine() generates value
  KM   -> KM   : format JSON string
  KM  --> DEV  : copy_to_user()
  DEV --> DR   : returns JSON string
  DR   -> SP   : parse(json)
  SP  --> DR   : SensorData
  DR   -> CB   : push(SensorData)
  DR   -> LOG  : log(SensorData)
  Main -> CB   : range(64)
  CB  --> Main : vector<SensorData>
  Main -> DASH : render()
  DASH -> DASH : draw current, graph, stats, alerts
end

User  -> Main  : SIGINT (Ctrl+C)
Main  -> DR    : stop()
Main  -> LOG   : close()
Main  -> DASH  : cleanup()

@enduml
```

---

## 3. State Machine Diagram – DeviceReader

```plantuml
@startuml VSensor_StateMachine

[*] --> Idle : constructed

Idle --> Opening : start() called

Opening --> Reading : open("/dev/vsensor") success
Opening --> Error   : open() fails (ENOENT / EACCES)

Reading --> Reading    : read() OK → parse → push → log
Reading --> Error      : read() returns -1 (EIO)
Reading --> Stopping   : stop() called

Error --> Opening : retry after backoff (max 5 retries)
Error --> Stopped : max retries exceeded

Stopping --> Stopped : thread joins, fd closed

Stopped --> [*]

@enduml
```

---

## 4. State Machine Diagram – Kernel Module Lifecycle

```plantuml
@startuml KernelModule_StateMachine

[*] --> Unloaded

Unloaded --> Initializing : insmod vsensor.ko

Initializing --> Ready     : alloc_chrdev_region OK\ndevice_create OK
Initializing --> Failed    : any init step fails

Ready --> Serving    : userspace open()
Serving --> Serving  : read / write / ioctl calls
Serving --> Ready    : release() called (all fds closed)

Ready --> Unloading  : rmmod vsensor
Serving --> Unloading : rmmod (forced)

Unloading --> Unloaded : cdev_del, unregister, class_destroy

Failed --> Unloaded : cleanup called in init error path

Unloaded --> [*]

@enduml
```
