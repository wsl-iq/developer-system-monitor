# Developer System Monitor

<div align="center">

**Professional Windows System Monitoring and Management Application**

A native Windows monitoring platform built with **C++20**, **Python 3.11+**, and **Pure Vanilla JavaScript (ES2020+)**.

<br>

[![Version](https://img.shields.io/badge/version-1.0.0-blue.svg)](#)
[![Platform](https://img.shields.io/badge/platform-Windows%2010%2F11-lightgrey.svg)](#)
[![C++](https://img.shields.io/badge/C%2B%2B-20-00599C.svg)](#)
[![Python](https://img.shields.io/badge/Python-3.11%2B-3776AB.svg)](#)
[![JavaScript](https://img.shields.io/badge/JavaScript-ES2020%2B-F7DF1E.svg)](#)
[![Architecture](https://img.shields.io/badge/architecture-C%2B%2B%20%7C%20Python%20%7C%20JavaScript-purple.svg)](#)
[![License](https://img.shields.io/badge/license-MIT-green.svg)](LICENSE)

</div>

---

## Overview

**Developer System Monitor** is a Windows system monitoring and management application designed for developers and advanced users.

The project combines a low-level native engine with a Python backend and a lightweight browser-based dashboard:

- **C++20** handles native Windows system interaction through Win32 and related Windows APIs.
- **Python 3.11+ / FastAPI** provides the backend, API layer, data processing, monitoring services, and native-library integration.
- **Pure Vanilla JavaScript (ES2020+)** provides the frontend dashboard and real-time visualization without a frontend framework.

The architecture is intentionally separated into independent layers so native operations, backend services, and the user interface remain maintainable and extensible.

---

## Features

### Real-Time System Monitoring

| Component | Monitoring |
|---|---|
| CPU | Usage, per-core metrics, frequency, temperature |
| Memory | Usage, availability, module information |
| GPU | Usage, VRAM, vendor information |
| Disk | Capacity, usage, read/write statistics |
| Network | Adapters, bandwidth, connection statistics |

### Process Management

- Full process enumeration
- CPU, memory, and thread information
- Process termination
- Process suspension and resumption
- Process priority management
- Executable path and user identification
- Search and sorting

### Windows Services

- Enumerate Windows services
- Start services
- Stop services
- Restart services
- View service status
- Manage startup type

### Hardware Information

- CPU model, cores, and frequency
- GPU vendor, VRAM, and driver information
- Memory module manufacturer, speed, and capacity
- Motherboard information
- BIOS information
- Storage devices

### Additional Capabilities

- Performance history
- Configurable alerts and thresholds
- JSON/HTML report generation
- Structured logging
- English/Arabic localization
- Dark/Light themes
- RTL support for Arabic

---

## Architecture

```text
┌──────────────────────────────────────────────────────────────┐
│                        Web Browser                           │
│                                                              │
│              Pure Vanilla JavaScript Frontend               │
│          Dashboard • Charts • Management • Settings          │
└──────────────────────────────┬───────────────────────────────┘
                               │
                         HTTP / WebSocket
                               │
                               ▼
┌──────────────────────────────────────────────────────────────┐
│                    Python Backend / FastAPI                  │
│                                                              │
│       REST API • WebSocket • Processing • Alerts             │
│       Reports • Configuration • Native Interface             │
└──────────────────────────────┬───────────────────────────────┘
                               │
                             ctypes
                               │
                               ▼
┌──────────────────────────────────────────────────────────────┐
│                    C++20 Native Core                         │
│                                                              │
│ Process • CPU • GPU • Memory • Disk • Network • Services     │
│                         Hardware                             │
└──────────────────────────────┬───────────────────────────────┘
                               │
                            Win32 API
                               │
                               ▼
┌──────────────────────────────────────────────────────────────┐
│                     Windows Operating System                 │
│          Win32 • PDH • WMI • DXGI • Registry • SCM           │
└──────────────────────────────────────────────────────────────┘
```

### Layer Responsibilities

#### C++ Native Core

**Location:** `cpp/`  
**Standard:** C++20  
**Build system:** CMake  
**Output:** `SystemMonitorNative.dll`

Responsible for low-level Windows operations and hardware/system data collection.

#### Python Backend

**Location:** `python/`  
**Version:** Python 3.11+  
**Framework:** FastAPI

Responsible for:

- Native DLL integration through `ctypes`
- REST API
- WebSocket communication
- Data normalization
- Monitoring services
- History storage
- Alert processing
- Report generation

#### Frontend

**Location:** `web/`  
**Language:** Pure Vanilla JavaScript (ES2020+)  
**Framework:** None

Responsible for:

- Dashboard
- Real-time visualization
- Process management interface
- Hardware/system views
- Settings
- Themes
- Localization

---

## Project Structure

```text
DeveloperSystemMonitor/
│
├── README.md
├── LICENSE
├── .gitignore
├── CMakeLists.txt
├── requirements.txt
├── pyproject.toml
│
├── cpp/
│   ├── CMakeLists.txt
│   ├── include/
│   │   ├── cpu.hpp
│   │   ├── memory.hpp
│   │   ├── process.hpp
│   │   ├── disk.hpp
│   │   ├── network.hpp
│   │   ├── services.hpp
│   │   ├── hardware.hpp
│   │   ├── system.hpp
│   │   └── exports.hpp
│   │
│   └── src/
│       ├── cpu.cpp
│       ├── memory.cpp
│       ├── process.cpp
│       ├── disk.cpp
│       ├── network.cpp
│       ├── services.cpp
│       ├── hardware.cpp
│       ├── system.cpp
│       └── exports.cpp
│
├── python/
│   ├── main.py
│   ├── __init__.py
│   ├── api/
│   │   ├── routes_system.py
│   │   ├── routes_process.py
│   │   ├── routes_disk.py
│   │   ├── routes_network.py
│   │   ├── routes_services.py
│   │   └── routes_settings.py
│   ├── core/
│   │   ├── native.py
│   │   ├── monitor.py
│   │   ├── process_manager.py
│   │   └── system_manager.py
│   ├── database/
│   │   ├── database.py
│   │   └── models.py
│   ├── services/
│   │   ├── monitoring.py
│   │   ├── alerts.py
│   │   ├── reports.py
│   │   └── automation.py
│   └── utils/
│       ├── logger.py
│       └── config.py
│
├── web/
│   ├── index.html
│   ├── css/
│   │   ├── main.css
│   │   ├── dashboard.css
│   │   ├── processes.css
│   │   └── settings.css
│   └── js/
│       ├── app.js
│       ├── api.js
│       ├── websocket.js
│       ├── dashboard.js
│       ├── processes.js
│       ├── system.js
│       ├── network.js
│       ├── services.js
│       ├── hardware.js
│       ├── logs.js
│       └── settings.js
│
├── scripts/
│   ├── build.bat
│   ├── run.bat
│   ├── clean.bat
│   └── copy_dlls.bat
│
└── docs/
    ├── architecture.md
    ├── api.md
    └── development.md
```

---

## Requirements

- Windows 10/11 (64-bit)
- MinGW-w64 with POSIX threads and SEH
- CMake 3.20+
- Python 3.11+
- Git (optional)

### Install Dependencies

#### MinGW-w64

```cmd
winget install -e --id BrechtSanders.WinLibs.POSIX.UCRT
```

#### CMake

```cmd
winget install Kitware.CMake
```

#### Python

```cmd
winget install Python.Python.3.11
```

#### Git

```cmd
winget install Git.Git
```

---

## Installation

### 1. Clone

```cmd
git clone https://github.com/wsl-iq/developer-system-monitor.git
cd developer-system-monitor
```

### 2. Build

```cmd
scripts\build.bat
```

### 3. Copy Runtime DLLs

```cmd
scripts\copy_dlls.bat
```

### 4. Run

```cmd
scripts\run.bat
```

Then open:

```text
http://127.0.0.1:8000
```

---

## API

The backend exposes a local REST API and a WebSocket endpoint.

### REST

| Method | Endpoint | Description |
|---|---|---|
| `GET` | `/api/health` | Health check |
| `GET` | `/api/system` | System information |
| `GET` | `/api/system/summary` | System summary |
| `GET` | `/api/cpu` | CPU information |
| `GET` | `/api/memory` | Memory information |
| `GET` | `/api/gpu` | GPU information |
| `GET` | `/api/hardware` | Hardware information |
| `GET` | `/api/disks` | Disk information |
| `GET` | `/api/network` | Network adapters |
| `GET` | `/api/network/stats` | Network statistics |
| `GET` | `/api/processes` | Process list |
| `GET` | `/api/processes/{pid}` | Process details |
| `POST` | `/api/processes/{pid}/terminate` | Terminate process |
| `POST` | `/api/processes/{pid}/suspend` | Suspend process |
| `POST` | `/api/processes/{pid}/resume` | Resume process |
| `GET` | `/api/services` | Service list |
| `POST` | `/api/services/{name}/start` | Start service |
| `POST` | `/api/services/{name}/stop` | Stop service |
| `POST` | `/api/services/{name}/restart` | Restart service |
| `GET` | `/api/settings` | Get settings |
| `PUT` | `/api/settings` | Update settings |
| `GET` | `/api/logs` | Get logs |

### WebSocket

```text
ws://127.0.0.1:8000/ws/monitor
```

Protocol:

```text
JSON over WebSocket
```

Default update interval:

```text
2 seconds
```

For complete API documentation, see [`docs/api.md`](docs/api.md).

---

## Security

The application is designed for local Windows usage.

- Backend binds to `127.0.0.1` by default.
- No external network exposure by default.
- API input validation.
- Process operations require confirmation.
- No arbitrary command execution.
- No remote-access functionality.
- No credential storage.

> **Note:** Process and service management operations can affect the operating system. Run the application with appropriate permissions and use administrative operations carefully.

---

## Development

Detailed development instructions are available in:

- [`docs/architecture.md`](docs/architecture.md)
- [`docs/api.md`](docs/api.md)
- [`docs/development.md`](docs/development.md)

### Build C++ Core

```cmd
cd cpp
cmake -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release
cmake --build .
```

### Run Backend

```cmd
cd python
python main.py
```

### Test Native Interface

```cmd
python python\test_native.py
```

### Test DLL Directly

```cmd
python python\test_dll_direct.py
```

---

## Performance

The project follows a lightweight monitoring strategy:

- Real-time sampling: approximately 1–5 seconds
- Database writes: batched every 5–10 seconds
- Hardware information: cached and refreshed approximately every 60 seconds
- Process list: refreshed every 2–5 seconds
- Chart history: limited to 30 points

---

## Roadmap

The following roadmap represents the intended project direction based on the current architecture:

- [ ] Complete native C++ monitoring modules
- [ ] Complete FastAPI route integration
- [ ] Complete WebSocket monitoring pipeline
- [ ] Expand hardware monitoring
- [ ] Improve process-management UI
- [ ] Expand Arabic localization
- [ ] Improve report generation
- [ ] Add automated tests
- [ ] Add release packaging
- [ ] Add CI build pipeline

---

## Documentation

| Document | Description |
|---|---|
| [`README.md`](README.md) | Project overview and quick start |
| [`docs/architecture.md`](docs/architecture.md) | System architecture and data flow |
| [`docs/api.md`](docs/api.md) | REST and WebSocket API |
| [`docs/development.md`](docs/development.md) | Development and troubleshooting |

---

## Contributing

Contributions are welcome.

Before submitting a pull request:

1. Create a feature branch.
2. Make your changes.
3. Follow the project coding standards.
4. Test the affected components.
5. Commit with a descriptive message.
6. Open a pull request with a clear description.

See [`docs/development.md`](docs/development.md) for development guidelines.

---

## License

This project is licensed under the **MIT License**.

See [`LICENSE`](LICENSE) for the full license text.

---

## Support

For bugs, feature requests, or technical discussions, use the project's GitHub Issues.

**Repository:** `wsl-iq/DeveloperSystemMonitor`

---

<div align="center">

**Developer System Monitor**

Built with C++20 • Python 3.11+ • Pure Vanilla JavaScript

</div>
