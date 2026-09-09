#!/usr/bin/env python3
# -*- coding: utf-8 -*-
# Copyright (c) 2026, lnc.
# Developer : Mohammed Al-Baqer

import asyncio
import json; json.encoder.FLOAT_REPR = lambda o: format(o, '.2f')  # Format floats to 2 decimal places
import logging
import time
from datetime import datetime
from typing import Any, Dict, List, Optional, Callable
from dataclasses import dataclass, asdict

from .native import NativeInterface, get_native_interface
logger = logging.getLogger(__name__)

@dataclass
class CPUMetrics:
    usage_percent: float
    per_core_usage: List[float]
    frequency_mhz: int
    temperature_celsius: Optional[float]
    timestamp: float


@dataclass
class MemoryMetrics:
    total_bytes: int
    available_bytes: int
    used_bytes: int
    usage_percent: float
    timestamp: float


@dataclass
class DiskMetrics:
    drive_letter: str
    total_bytes: int
    free_bytes: int
    used_bytes: int
    usage_percent: float
    read_bytes_per_sec: int
    write_bytes_per_sec: int
    timestamp: float


@dataclass
class NetworkMetrics:
    adapter_name: str
    bytes_received: int
    bytes_sent: int
    bytes_received_per_sec: int
    bytes_sent_per_sec: int
    timestamp: float


class SystemMonitor:
    def __init__(self, native: Optional[NativeInterface] = None, interval: float = 2.0):
        self.native = native or get_native_interface()
        self.interval = interval
        self._running = False
        self._task: Optional[asyncio.Task] = None
        self._callbacks: List[Callable] = []
        self._latest_metrics: Dict[str, Any] = {}
        
        self._previous_network_data = {}
        self._previous_disk_data = {}
        self._last_network_sample_time = time.time()
        self._last_disk_sample_time = time.time()
    
    async def start(self):
        if self._running:
            logger.warning("Monitor is already running")
            return
        
        self._running = True
        self._task = asyncio.create_task(self._monitoring_loop())
        logger.info(f"System monitor started with interval {self.interval}s")
    
    async def stop(self):
        if not self._running:
            return
        
        self._running = False
        if self._task:
            self._task.cancel()
            try:
                await self._task
            except asyncio.CancelledError:
                pass
        
        logger.info("System monitor stopped")
    
    async def _monitoring_loop(self):
        while self._running:
            try:
                metrics = await self.collect_metrics()
                self._latest_metrics = metrics
                
                # Notify callbacks
                for callback in self._callbacks:
                    try:
                        if asyncio.iscoroutinefunction(callback):
                            await callback(metrics)
                        else:
                            callback(metrics)
                    except Exception as e:
                        logger.error(f"Callback error: {e}")
                
                await asyncio.sleep(self.interval)
            except asyncio.CancelledError:
                break
            except Exception as e:
                logger.error(f"Monitoring loop error: {e}")
                await asyncio.sleep(self.interval)
    
    async def collect_metrics(self) -> Dict[str, Any]:
        metrics = {
            'timestamp': datetime.now().isoformat(),
            'cpu': self.get_cpu_metrics(),
            'memory': self.get_memory_metrics(),
            'disks': self.get_disk_metrics(),
            'network': self.get_network_metrics(),
        }
        
        return metrics
    
    def get_cpu_metrics(self) -> Dict[str, Any]:
        cpu_info = self.native.get_cpu_info()
        
        if not cpu_info:
            return {
                'usage_percent': 0,
                'per_core_usage': [],
                'frequency_mhz': 0,
                'temperature_celsius': None,
            }
        
        return {
            'usage_percent': cpu_info.get('total_usage_percent', 0),
            'per_core_usage': cpu_info.get('core_usages', []),
            'frequency_mhz': cpu_info.get('current_frequency_mhz', 0),
            'temperature_celsius': cpu_info.get('temperature_celsius'),
        }
    
    def get_memory_metrics(self) -> Dict[str, Any]:
        memory_info = self.native.get_memory_info()
        
        if not memory_info:
            return {
                'total_bytes': 0,
                'available_bytes': 0,
                'used_bytes': 0,
                'usage_percent': 0,
            }
        
        return {
            'total_bytes': memory_info.get('total_physical_bytes', 0),
            'available_bytes': memory_info.get('available_physical_bytes', 0),
            'used_bytes': memory_info.get('used_physical_bytes', 0),
            'usage_percent': memory_info.get('usage_percent', 0),
        }
    
    def get_disk_metrics(self) -> List[Dict[str, Any]]:
        current_time = time.time()
        disks = self.native.get_disk_info() or []
        
        result = []
        for disk in disks:
            drive = disk.get('drive_letter', '')
            
            # Calculate read/write rates
            read_rate = 0
            write_rate = 0
            
            if drive in self._previous_disk_data:
                time_delta = current_time - self._last_disk_sample_time
                if time_delta > 0:
                    read_rate = (disk.get('total_read_bytes', 0) - 
                                self._previous_disk_data[drive].get('total_read_bytes', 0)) / time_delta
                    write_rate = (disk.get('total_write_bytes', 0) - 
                                 self._previous_disk_data[drive].get('total_write_bytes', 0)) / time_delta
            
            self._previous_disk_data[drive] = disk
            
            result.append({
                'drive_letter': drive,
                'total_bytes': disk.get('total_bytes', 0),
                'free_bytes': disk.get('free_bytes', 0),
                'used_bytes': disk.get('used_bytes', 0),
                'usage_percent': disk.get('usage_percent', 0),
                'read_bytes_per_sec': int(read_rate),
                'write_bytes_per_sec': int(write_rate),
            })
        
        self._last_disk_sample_time = current_time
        return result
    
    def get_network_metrics(self) -> List[Dict[str, Any]]:
        current_time = time.time()
        adapters = self.native.get_network_info() or []
        
        result = []
        for adapter in adapters:
            name = adapter.get('name', '')
            
            # Calculate rates
            recv_rate = 0
            sent_rate = 0
            
            if name in self._previous_network_data:
                time_delta = current_time - self._last_network_sample_time
                if time_delta > 0:
                    recv_rate = (adapter.get('bytes_received', 0) - 
                                self._previous_network_data[name].get('bytes_received', 0)) / time_delta
                    sent_rate = (adapter.get('bytes_sent', 0) - 
                                self._previous_network_data[name].get('bytes_sent', 0)) / time_delta
            
            self._previous_network_data[name] = adapter
            
            result.append({
                'adapter_name': name,
                'description': adapter.get('description', ''),
                'bytes_received': adapter.get('bytes_received', 0),
                'bytes_sent': adapter.get('bytes_sent', 0),
                'bytes_received_per_sec': int(recv_rate),
                'bytes_sent_per_sec': int(sent_rate),
                'status': adapter.get('status', 'Unknown'),
            })
        
        self._last_network_sample_time = current_time
        return result
    
    def get_latest_metrics(self) -> Dict[str, Any]:
        return self._latest_metrics
    
    def add_callback(self, callback: Callable):
        if callback not in self._callbacks:
            self._callbacks.append(callback)
    
    def remove_callback(self, callback: Callable):
        if callback in self._callbacks:
            self._callbacks.remove(callback)

    def jsonify_metrics(self, metrics: Dict[str, Any]) -> str:
        options = {
            'indent': 4,
            'sort_keys': True,
            'default': str,  # Convert non-serializable objects to strings
        }
        return json.dumps(metrics, **options)