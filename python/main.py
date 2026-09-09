#!/usr/bin/env python3
# -*- coding: utf-8 -*-
# Copyright (c) 2026, lnc.
# Developer : Mohammed Al-Baqer

import logging
import sys
import os
import traceback
from pathlib import Path

# Add project root to Python path
project_root = Path(__file__).parent.parent
sys.path.insert(0, str(project_root))

# Add MinGW bin to PATH for DLL dependencies
mingw_paths = [
    r"C:\mingw64\bin",
    r"C:\msys64\mingw64\bin",
    r"C:\MinGW\bin",
]
for path in mingw_paths:
    if os.path.exists(path):
        os.environ['PATH'] = path + ';' + os.environ['PATH']
        break

from python.utils.logger import setup_logging
from python.utils.config import Config

logger = logging.getLogger(__name__)


def main():    
    # Set up logging
    setup_logging()
    
    logger.info("=" * 50)
    logger.info("Developer System Monitor Starting")
    logger.info("=" * 50)
    
    # Load configuration
    config = Config()
    
    # Initialize native interface
    native = None
    try:
        from python.core.native import NativeInterface
        native = NativeInterface()
        
        if native.is_available():
            logger.info("Native C++ library loaded successfully")
        else:
            logger.warning("Native C++ library not available")
    except Exception as e:
        logger.error(f"Failed to initialize native interface: {e}")
    
    # Start FastAPI server
    try:
        import uvicorn
        from fastapi import FastAPI, WebSocket, WebSocketDisconnect, Query, HTTPException
        from fastapi.staticfiles import StaticFiles
        from fastapi.middleware.cors import CORSMiddleware
        from fastapi.responses import FileResponse, JSONResponse
        import asyncio
        import json
        from datetime import datetime
        from typing import Optional, List, Dict, Any
        
        # Create FastAPI app
        app = FastAPI(
            title="Developer System Monitor",
            description="Professional Windows system monitoring application",
            version="1.0.0",
        )
        
        # Add CORS middleware
        app.add_middleware(
            CORSMiddleware,
            allow_origins=["*"],
            allow_credentials=True,
            allow_methods=["*"],
            allow_headers=["*"],
        )
        
        # Helper function to safely call native functions
        def safe_native_call(func_name, default=None):
            """Safely call a native function with error handling."""
            if native and native.is_available():
                try:
                    result = getattr(native, func_name)()
                    return result if result is not None else default
                except Exception as e:
                    logger.error(f"Native call {func_name} failed: {e}")
                    return default
            return default
        
        # System Endpoints
        
        @app.get("/api/health")
        async def health_check():
            return {
                "status": "ok",
                "native_available": native.is_available() if native else False,
                "timestamp": datetime.now().isoformat()
            }
        
        @app.get("/api/system")
        async def get_system():
            result = safe_native_call('get_system_info', {})
            if not result:
                result = {
                    "hostname": "Unknown",
                    "os_name": "Unknown",
                    "os_version": "Unknown",
                    "os_build": "Unknown",
                    "architecture": "Unknown",
                    "uptime_seconds": 0,
                    "processor_count": 0
                }
            return result
        
        @app.get("/api/system/summary")
        async def get_system_summary():
            system = safe_native_call('get_system_info', {})
            cpu = safe_native_call('get_cpu_info', {})
            memory = safe_native_call('get_memory_info', {})
            
            return {
                "hostname": system.get("hostname", "Unknown"),
                "os_name": system.get("os_name", "Unknown"),
                "os_version": system.get("os_version", "Unknown"),
                "uptime_seconds": system.get("uptime_seconds", 0),
                "architecture": system.get("architecture", "Unknown"),
                "cpu_model": cpu.get("model", "Unknown"),
                "cpu_usage": cpu.get("total_usage_percent", 0),
                "memory_usage": memory.get("usage_percent", 0),
                "memory_total_gb": round(memory.get("total_physical_bytes", 0) / (1024**3), 2),
            }
        
        @app.get("/api/cpu")
        async def get_cpu():
            result = safe_native_call('get_cpu_info', {})
            if not result:
                result = {
                    "manufacturer": "Unknown",
                    "model": "Unknown",
                    "architecture": "Unknown",
                    "physical_cores": 0,
                    "logical_processors": 0,
                    "current_frequency_mhz": 0,
                    "max_frequency_mhz": 0,
                    "total_usage_percent": 0,
                    "core_usages": [],
                }
            return result
        
        @app.get("/api/memory")
        async def get_memory():
            result = safe_native_call('get_memory_info', {})
            if not result:
                result = {
                    "total_physical_bytes": 0,
                    "available_physical_bytes": 0,
                    "used_physical_bytes": 0,
                    "usage_percent": 0,
                }
            return result
        
        @app.get("/api/gpu")
        async def get_gpu():
            hardware = safe_native_call('get_hardware_info', {})
            gpus = hardware.get("gpus", []) if hardware else []
            return {"gpus": gpus, "count": len(gpus)}
        
        @app.get("/api/hardware")
        async def get_hardware():
            result = safe_native_call('get_hardware_info', {})
            return result or {}
        
        @app.get("/api/metrics/current")
        async def get_current_metrics():
            cpu = safe_native_call('get_cpu_info', {})
            memory = safe_native_call('get_memory_info', {})
            disks = safe_native_call('get_disk_info', [])
            network = safe_native_call('get_network_info', [])
            
            # Calculate network totals
            total_download = 0
            total_upload = 0
            if isinstance(network, list):
                for adapter in network:
                    total_download += adapter.get("bytes_received", 0)
                    total_upload += adapter.get("bytes_sent", 0)
            
            return {
                "timestamp": datetime.now().isoformat(),
                "cpu": {
                    "usage_percent": cpu.get("total_usage_percent", 0),
                    "frequency_mhz": cpu.get("current_frequency_mhz", 0),
                    "temperature_celsius": cpu.get("temperature_celsius"),
                },
                "memory": {
                    "total_bytes": memory.get("total_physical_bytes", 0),
                    "used_bytes": memory.get("used_physical_bytes", 0),
                    "available_bytes": memory.get("available_physical_bytes", 0),
                    "usage_percent": memory.get("usage_percent", 0),
                },
                "gpu": {
                    "usage_percent": 0,
                    "name": "GPU",
                },
                "disks": disks if isinstance(disks, list) else [],
                "network": {
                    "total_download_bytes": total_download,
                    "total_upload_bytes": total_upload,
                    "adapters": network if isinstance(network, list) else [],
                }
            }
        
        # Process Endpoints
        
        @app.get("/api/processes")
        async def get_processes(
            search: Optional[str] = Query(None),
            sort_by: Optional[str] = Query(None),
            sort_order: Optional[str] = Query("asc"),
            limit: Optional[int] = Query(None),
        ):
            processes = safe_native_call('get_process_list', [])
            
            if not isinstance(processes, list):
                processes = []
            
            # Apply search
            if search:
                search_lower = search.lower()
                processes = [p for p in processes if 
                           search_lower in str(p.get("name", "")).lower() or search_lower in str(p.get("pid", "")).lower()]
            
            # Apply sort
            if sort_by:
                reverse = sort_order.lower() == "desc"
                try:
                    processes.sort(key=lambda x: x.get(sort_by, 0), reverse=reverse)
                except:
                    pass
            
            # Apply limit
            if limit and limit > 0:
                processes = processes[:limit]
            
            return processes
        
        @app.get("/api/processes/stats/summary")
        async def get_process_stats():
            processes = safe_native_call('get_process_list', [])
            
            if not isinstance(processes, list):
                processes = []
            
            total_memory = sum(p.get("memory_usage_bytes", 0) for p in processes)
            total_cpu = sum(p.get("cpu_usage_percent", 0) for p in processes)
            
            return {
                "total_processes": len(processes),
                "total_memory_bytes": total_memory,
                "total_cpu_percent": round(total_cpu, 2),
            }
        
        @app.get("/api/processes/{pid}")
        async def get_process(pid: int):
            processes = safe_native_call('get_process_list', [])
            if isinstance(processes, list):
                for process in processes:
                    if process.get("pid") == pid:
                        return process
            
            raise HTTPException(status_code=404, detail=f"Process {pid} not found")
        
        @app.post("/api/processes/{pid}/terminate")
        async def terminate_process(pid: int):
            if native and native.is_available():
                success = native.terminate_process(pid)
                if success:
                    return {"success": True, "message": f"Process {pid} terminated"}
                raise HTTPException(status_code=400, detail=f"Failed to terminate process {pid}")
            raise HTTPException(status_code=503, detail="Native library not available")
        
        @app.post("/api/processes/{pid}/suspend")
        async def suspend_process(pid: int):
            if native and native.is_available():
                success = native.suspend_process(pid)
                if success:
                    return {"success": True, "message": f"Process {pid} suspended"}
                raise HTTPException(status_code=400, detail=f"Failed to suspend process {pid}")
            raise HTTPException(status_code=503, detail="Native library not available")
        
        @app.post("/api/processes/{pid}/resume")
        async def resume_process(pid: int):
            if native and native.is_available():
                success = native.resume_process(pid)
                if success:
                    return {"success": True, "message": f"Process {pid} resumed"}
                raise HTTPException(status_code=400, detail=f"Failed to resume process {pid}")
            raise HTTPException(status_code=503, detail="Native library not available")
        
        # Disk Endpoints
        
        @app.get("/api/disks")
        async def get_disks():
            result = safe_native_call('get_disk_info', [])
            return result if isinstance(result, list) else []
        
        @app.get("/api/disks/usage")
        async def get_disk_usage():
            disks = safe_native_call('get_disk_info', [])
            if not isinstance(disks, list):
                return []
            
            usage_data = []
            for disk in disks:
                usage_data.append({
                    "drive_letter": disk.get("drive_letter", ""),
                    "volume_name": disk.get("volume_name", ""),
                    "total_bytes": disk.get("total_bytes", 0),
                    "free_bytes": disk.get("free_bytes", 0),
                    "used_bytes": disk.get("used_bytes", 0),
                    "usage_percent": disk.get("usage_percent", 0),
                    "file_system": disk.get("file_system", ""),
                    "drive_type": disk.get("drive_type", ""),
                })
            
            return usage_data
        
        # Network Endpoints
        
        @app.get("/api/network")
        async def get_network():
            result = safe_native_call('get_network_info', [])
            return result if isinstance(result, list) else []
        
        @app.get("/api/network/stats")
        async def get_network_stats():
            adapters = safe_native_call('get_network_info', [])
            
            if not isinstance(adapters, list):
                return {"total_adapters": 0, "active_adapters": 0, 
                        "total_bytes_received": 0, "total_bytes_sent": 0}
            
            return {
                "total_adapters": len(adapters),
                "active_adapters": sum(1 for a in adapters if a.get("status") == "Up"),
                "total_bytes_received": sum(a.get("bytes_received", 0) for a in adapters),
                "total_bytes_sent": sum(a.get("bytes_sent", 0) for a in adapters),
            }
        
        # Services Endpoints
        
        @app.get("/api/services")
        async def get_services():
            result = safe_native_call('get_service_list', [])
            return result if isinstance(result, list) else []
        
        @app.post("/api/services/{service_name}/start")
        async def start_service(service_name: str):
            if native and native.is_available():
                success = native.start_service(service_name)
                if success:
                    return {"success": True, "message": f"Service {service_name} started"}
                raise HTTPException(status_code=400, detail=f"Failed to start service {service_name}")
            raise HTTPException(status_code=503, detail="Native library not available")
        
        @app.post("/api/services/{service_name}/stop")
        async def stop_service(service_name: str):
            if native and native.is_available():
                success = native.stop_service(service_name)
                if success:
                    return {"success": True, "message": f"Service {service_name} stopped"}
                raise HTTPException(status_code=400, detail=f"Failed to stop service {service_name}")
            raise HTTPException(status_code=503, detail="Native library not available")
        
        @app.post("/api/services/{service_name}/restart")
        async def restart_service(service_name: str):
            if native and native.is_available():
                success = native.restart_service(service_name)
                if success:
                    return {"success": True, "message": f"Service {service_name} restarted"}
                raise HTTPException(status_code=400, detail=f"Failed to restart service {service_name}")
            raise HTTPException(status_code=503, detail="Native library not available")
        
        # Settings Endpoints
        
        @app.get("/api/settings")
        async def get_settings():
            return config.get_all()
        
        @app.put("/api/settings")
        async def update_settings(request_data: dict):
            updates = request_data.get("updates", request_data)
            config.update(updates)
            return {"success": True, "message": "Settings updated", "settings": config.get_all()}
        
        @app.post("/api/settings/reset")
        async def reset_settings():
            config.reset()
            return {"success": True, "message": "Settings reset", "settings": config.get_all()}
        
        # Logs Endpoints
        
        @app.get("/api/logs")
        async def get_logs():
            logs = []
            log_dir = project_root / 'python' / 'logs'
            
            if log_dir.exists():
                for log_file in log_dir.glob('*.log'):
                    try:
                        with open(log_file, 'r', encoding='utf-8', errors='ignore') as f:
                            lines = f.readlines()
                            for line in lines[-200:]:  # Last 200 lines
                                line = line.strip()
                                if line:
                                    level = "INFO"
                                    if "ERROR" in line:
                                        level = "ERROR"
                                    elif "WARNING" in line:
                                        level = "WARNING"
                                    
                                    logs.append({
                                        "timestamp": datetime.now().strftime("%H:%M:%S"),
                                        "level": level,
                                        "message": line,
                                    })
                    except Exception as e:
                        pass
            
            return logs
        
        @app.delete("/api/logs")
        async def clear_logs():
            log_dir = project_root / 'python' / 'logs'
            if log_dir.exists():
                for log_file in log_dir.glob('*.log'):
                    try:
                        with open(log_file, 'w') as f:
                            f.write('')
                    except:
                        pass
            return {"success": True, "message": "Logs cleared"}
        
        # WebSocket Endpoint
        
        @app.websocket("/ws/monitor")
        async def websocket_monitor(websocket: WebSocket):
            await websocket.accept()
            logger.info("WebSocket client connected")
            
            try:
                while True:
                    try:
                        # Get all metrics
                        cpu = safe_native_call('get_cpu_info', {})
                        memory = safe_native_call('get_memory_info', {})
                        disks = safe_native_call('get_disk_info', [])
                        network = safe_native_call('get_network_info', [])
                        
                        # Calculate network totals
                        total_download = 0
                        total_upload = 0
                        if isinstance(network, list):
                            for adapter in network:
                                total_download += adapter.get("bytes_received_per_sec", 0) or adapter.get("bytes_received", 0)
                                total_upload += adapter.get("bytes_sent_per_sec", 0) or adapter.get("bytes_sent", 0)
                        
                        # Build metrics payload
                        metrics = {
                            "timestamp": datetime.now().isoformat(),
                            "cpu": {
                                "usage_percent": cpu.get("total_usage_percent", 0) if cpu else 0,
                                "frequency_mhz": cpu.get("current_frequency_mhz", 0) if cpu else 0,
                                "temperature_celsius": cpu.get("temperature_celsius") if cpu else None,
                            },
                            "memory": {
                                "total_bytes": memory.get("total_physical_bytes", 0) if memory else 0,
                                "used_bytes": memory.get("used_physical_bytes", 0) if memory else 0,
                                "available_bytes": memory.get("available_physical_bytes", 0) if memory else 0,
                                "usage_percent": memory.get("usage_percent", 0) if memory else 0,
                            },
                            "gpu": {
                                "usage_percent": 0,
                                "name": "GPU",
                            },
                            "disks": disks if isinstance(disks, list) else [],
                            "network": network if isinstance(network, list) else [],
                            "network_totals": {
                                "download": total_download,
                                "upload": total_upload,
                            }
                        }
                        
                        await websocket.send_json(metrics)
                        
                        # Wait for next update
                        interval = config.get('monitoring', 'interval_seconds', 2)
                        await asyncio.sleep(interval)
                        
                    except WebSocketDisconnect:
                        raise
                    except Exception as e:
                        logger.error(f"WebSocket error: {e}")
                        await asyncio.sleep(2)
                        
            except WebSocketDisconnect:
                logger.info("WebSocket client disconnected")
        
        # Static Files
        
        web_dir = project_root / 'web'
        if web_dir.exists():
            if (web_dir / 'css').exists():
                app.mount("/css", StaticFiles(directory=str(web_dir / 'css')), name="css")
            if (web_dir / 'js').exists():
                app.mount("/js", StaticFiles(directory=str(web_dir / 'js')), name="js")
            
            @app.get("/")
            async def serve_index():
                index_file = web_dir / 'index.html'
                if index_file.exists():
                    return FileResponse(str(index_file))
                return {"message": "Developer System Monitor API", "docs": "/docs"}
        
        # Get server config
        host = config.get('server', 'host', '127.0.0.1')
        port = config.get('server', 'port', 8000)
        
        logger.info("=" * 50)
        logger.info(f"Server running at: http://{host}:{port}")
        logger.info(f"API docs at: http://{host}:{port}/docs")
        logger.info("=" * 50)
        
        # Run server
        uvicorn.run(app, host=host, port=port, log_level="info")
        
    except Exception as e:
        logger.error(f"Server error: {e}")
        logger.error(traceback.format_exc())
        
        logger.info("Running in test mode - press Ctrl+C to exit")
        try:
            import time
            while True:
                time.sleep(1)
        except KeyboardInterrupt:
            pass
    
    logger.info("Application shutdown complete")

if __name__ == "__main__":
    main()