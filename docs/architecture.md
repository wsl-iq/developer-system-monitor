# Architecture Documentation

## Overview

Developer System Monitor uses a three-layer architecture that separates concerns and leverages the strengths of three programming languages.

## System Architecture

```
┌─────────────────────────────────────────────────────────┐
│ Web Browser                                             │
│ ┌───────────────────────────────────────────────────┐   │
│ │ JavaScript Frontend                               │   │
│ │ - Dashboard - Processes                           │   │
│ │ - Charts (Chart.js)                               │   │
│ │ - Real-time updates via WebSocket                 │   │
│ └───────────────────────────────────────────────────┘   │
└─────────────────────────────────────────────────────────┘
│
│ HTTP / WebSocket
▼
┌─────────────────────────────────────────────────────────┐
│ Python Backend (FastAPI)                                │
│ ┌───────────────────────────────────────────────────┐   │
│ │ - REST API                                        │   │
│ │ - WebSocket Server                                │   │
│ │ - Data Processing                                 │   │
│ │ - Configuration Management                        │   │
│ │ - Alert System                                    │   │
│ └───────────────────────────────────────────────────┘   │
└─────────────────────────────────────────────────────────┘
│
│ ctypes (DLL calls)
▼
┌─────────────────────────────────────────────────────────┐
│ C++ Core (SystemMonitorNative.dll)                      │
│ ┌───────────────────────────────────────────────────┐   │
│ │ - Process Management                              │   │
│ │ - CPU/GPU Monitoring                              │   │
│ │ - Memory Management                               │   │
│ │ - Disk/Network Statistics                         │   │
│ │ - Windows Services                                │   │
│ │ - Hardware Information                            │   │
│ └───────────────────────────────────────────────────┘   │
└─────────────────────────────────────────────────────────┘
│
│ Win32 API
▼
┌─────────────────────────────────────────────────────────┐
│ Windows Operating System                                │
│ ┌───────────────────────────────────────────────────┐   │
│ │ - Process APIs - PDH Counters                     │   │
│ │ - WMI - DXGI                                      │   │
│ │ - Registry - Service Control Manager              │   │
│ └───────────────────────────────────────────────────┘   │
└─────────────────────────────────────────────────────────┘
```
## Communication Flow

### Data Flow (Reading)

```
1. C++ queries Windows APIs (PDH, WMI, Registry, etc.)
2. C++ formats data as JSON strings
3. C++ returns JSON to Python via ctypes
4. Python parses and normalizes JSON
5. Python sends to JavaScript via WebSocket/REST
6. JavaScript renders data in UI
```
### Command Flow (Actions)

```
1. User interacts with JavaScript UI
2. JavaScript sends REST request to Python
3. Python validates input
4. Python calls C++ function via ctypes
5. C++ executes Windows API call
6. Result propagates back through layers
```
## Component Details

### C++ Core Components

| Component | File | Responsibility |
|-----------|------|----------------|
| CPUMonitor | `cpu.cpp` | CPU info, usage, frequency, temperature |
| MemoryMonitor | `memory.cpp` | Memory info, modules, usage |
| ProcessManager | `process.cpp` | Process enumeration, management |
| DiskMonitor | `disk.cpp` | Disk info, usage, capacity |
| NetworkMonitor | `network.cpp` | Adapter info, traffic stats |
| ServiceManager | `services.cpp` | Windows service management |
| HardwareMonitor | `hardware.cpp` | Hardware info via WMI/DXGI/Registry |
| SystemMonitor | `system.cpp` | OS info, uptime, architecture |

### Python Components

| Component | File | Responsibility |
|-----------|------|----------------|
| NativeInterface | `core/native.py` | ctypes wrapper for C++ DLL |
| SystemManager | `core/system_manager.py` | High-level system operations |
| ProcessManager | `core/process_manager.py` | Process management wrapper |
| Monitor | `core/monitor.py` | Real-time monitoring service |
| Database | `database/database.py` | SQLite data persistence |
| AlertService | `services/alerts.py` | Alert rule management |
| ReportGenerator | `services/reports.py` | Report generation |

### JavaScript Components

| Component | File | Responsibility |
|-----------|------|----------------|
| App | `js/app.js` | Main controller, navigation, themes |
| Dashboard | `js/dashboard.js` | Real-time metrics, charts |
| ProcessManager | `js/processes.js` | Process UI |
| SystemViewer | `js/system.js` | CPU/Memory/GPU/Disk views |
| NetworkViewer | `js/network.js` | Network monitoring UI |
| ServicesManager | `js/services.js` | Service management UI |
| HardwareViewer | `js/hardware.js` | Hardware info display |
| LogViewer | `js/logs.js` | Log viewing |
| SettingsManager | `js/settings.js` | Settings UI |

## Performance Considerations

- **Sampling Intervals**: 1-5 seconds for real-time metrics
- **Database Writes**: Batched, every 5-10 seconds
- **Hardware Info**: Cached, refreshed every 60 seconds
- **Process List**: Refreshed every 2-5 seconds
- **Chart Data Points**: Limited to 30 points for performance

## Security Considerations

- Local binding only (127.0.0.1)
- Input validation at Python layer
- Process operations require confirmation
- No remote command execution
- No credential storage
