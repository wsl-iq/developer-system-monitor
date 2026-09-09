`markdown`

# API Documentation
## Base URL
[http://127.0.0.1:8000](http://127.0.0.1:8000/)

## REST API Endpoints
### System

#### Get System Info
```http
GET /api/system
```

**Response:**
```json
{
    "hostname": "DESKTOP-ABC123",
    "os_name": "Windows 10 Pro",
    "os_version": "10.0",
    "os_build": "19045",
    "architecture": "x64",
    "uptime_seconds": 86400,
    "processor_count": 8
}
```

#### Get CPU Info

```http
GET /api/cpu
```

**Response:**

```json
{
    "manufacturer": "GenuineIntel",
    "model": "Intel(R) Core(TM) i7-9700K",
    "architecture": "x64",
    "physical_cores": 8,
    "logical_processors": 8,
    "current_frequency_mhz": 3600,
    "total_usage_percent": 25.5,
    "core_usages": [10.2, 15.3, 8.7, 12.1]
}
```

#### Get Memory Info
```http
GET /api/memory
```

**Response:**

```json
{
    "total_physical_bytes": 17179869184,
    "available_physical_bytes": 8589934592,
    "used_physical_bytes": 8589934592,
    "usage_percent": 50.0
}
```

### Processes
#### Get Process List

```http
GET /api/processes
```

**Query Parameters:**

- `search`: Filter by name or PID
- `sort_by`: Field to sort by
- `sort_order`: `asc` or `desc`
- `limit`: Maximum results

#### Get Process Details

```http
GET /api/processes/{pid}
```

#### Terminate Process

```http
POST /api/processes/{pid}/terminate
```

#### Suspend Process

```http
POST /api/processes/{pid}/suspend
```

#### Resume Process

```http
POST /api/processes/{pid}/resume
```

### Services
#### Get Service List

```http
GET /api/services
```

#### Start Service
```http
POST /api/services/{service_name}/start
```

#### Stop Service
```http
POST /api/services/{service_name}/stop
```

#### Restart Service
```http
POST /api/services/{service_name}/restart
```

### Settings
#### Get Settings
```http
GET /api/settings
```

#### Update Settings
```http
PUT /api/settings
```

**Request Body:**
```json
{
    "monitoring": {
        "interval_seconds": 2,
        "enable_gpu_monitoring": true
    },
    "alerts": {
        "enable_alerts": true,
        "cpu_threshold_percent": 90
    }
}
```

## WebSocket API
### Endpoint

`ws://127.0.0.1:8000/ws/monitor`

### Message Format

```json
{
    "timestamp": "2026-01-01T12:00:00",
    "cpu": {
        "usage_percent": 25.5,
        "frequency_mhz": 3600
    },
    "memory": {
        "usage_percent": 50.0,
        "total_bytes": 17179869184,
        "used_bytes": 8589934592
    },
    "gpu": {
        "usage_percent": 15.0
    },
    "disks": [
        {
            "drive_letter": "C:",
            "usage_percent": 75.5,
            "total_bytes": 107374182400,
            "free_bytes": 26843545600
        }
    ],
    "network": {
        "total_download": 125000,
        "total_upload": 32000
    }
}
```

## Error Handling

All endpoints return appropriate HTTP status codes:
- `200`: Success
- `400`: Bad request
- `404`: Not found
- `500`: Internal server error
- `503`: Service unavailable (native library not loaded)

## Rate Limiting
No rate limiting is implemented as the API is local-only. However, clients should respect reasonable polling intervals (1-5 seconds).
