# Development Guide

## Environment Setup

### Prerequisites

1. **MinGW-w64**
```cmd
winget install -e --id BrechtSanders.WinLibs.POSIX.UCRT
```


2. **CMake**
```cmd
   winget install Kitware.CMake
```

3. **Python 3.11+**
```cmd   
   winget install Python.Python.3.11
```

4. **Git** (optional)
```cmd
   winget install Git.Git
```

### Initial Setup

```cmd
git clone https://github.com/wsl-iq/DeveloperSystemMonitor.git
cd DeveloperSystemMonitor
scripts\build.bat
scripts\copy_dlls.bat
scripts\run.bat
```

## Project Structure
```
DeveloperSystemMonitor/
├── cpp/              # C++ native core
│   ├── include/      # Header files
│   └── src/          # Source files
├── python/           # Python backend
│   ├── api/          # API routes
│   ├── core/         # Core logic
│   ├── database/     # Database layer
│   ├── services/     # Business services
│   └── utils/        # Utilities
├── web/              # JavaScript frontend
│   ├── css/          # Stylesheets
│   └── js/           # JavaScript files
├── scripts/          # Build scripts
└── docs/             # Documentation
```
## Development Workflow

### C++ Development

1. Edit files in `cpp/include/` and `cpp/src/`
2. Build:

```cmd
cd cpp
cmake -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Debug
cmake --build .
```
   
3. Test:

```cmd
python python\test_native.py
```


### Python Development

1. Edit files in `python/`
2. Run directly:
```cmd
   cd python
   python main.py
```

3. Test API at `http://127.0.0.1:8000/docs`

### JavaScript Development

1. Edit files in `web/`
2. Open `http://127.0.0.1:8000` in browser
3. Use browser DevTools (F12) for debugging

## Coding Standards

### C++ (C++20)

- Use RAII for resource management
- Smart pointers over raw pointers
- `snake_case` for functions
- `PascalCase` for classes
- JSON escaping for all string outputs

### Python (3.11+)

- Follow PEP 8
- Type hints required
- Docstrings for public APIs
- Async where appropriate
- Error handling with try/except

### JavaScript (ES2020+)

- Use `const` and `let` (no `var`)
- Async/await for API calls
- Event delegation for DOM events
- No jQuery dependencies

## Testing

### Native Interface Test

```cmd
python python\test_native.py
```

### Direct DLL Test

```cmd
python python\test_dll_direct.py
```

### API Test

Open `http://127.0.0.1:8000/docs` and use Swagger UI

## Troubleshooting

### DLL Not Found

```cmd
scripts\copy_dlls.bat
```

### Build Errors

```cmd
scripts\clean.bat
scripts\build.bat
```

### Port Already in Use

Change port in `data/config.json` or kill process on port 8000

### Python Import Errors

```cmd
cd python
pip install -r requirements.txt
```

## Git Workflow

1. Create feature branch
2. Make changes
3. Test thoroughly
4. Commit with descriptive message
5. Push and create pull request

## Performance Guidelines

- Minimize native API calls
- Cache hardware info (refresh every 60s)
- Batch database writes
- Limit chart data points to 30
- Use WebSocket for real-time updates
