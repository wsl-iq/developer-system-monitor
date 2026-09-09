#!/usr/bin/env python3
# -*- coding: utf-8 -*-
# Copyright (c) 2026, lnc.
# Developer : Mohammed Al-Baqer

import logging
from typing import Any, Dict, List, Optional
from fastapi import APIRouter, HTTPException, Query
from pydantic import BaseModel, Field
from ..core.system_manager import SystemManager
from ..core.monitor import SystemMonitor

logger = logging.getLogger(__name__)
router = APIRouter(prefix="/api", tags=["system"])

# Initialize managers
system_manager = SystemManager()
system_monitor = SystemMonitor()

class SystemInfoResponse(BaseModel):
    system: Dict[str, Any]
    cpu: Dict[str, Any]
    memory: Dict[str, Any]
    disks: List[Dict[str, Any]]
    network: List[Dict[str, Any]]
    hardware: Dict[str, Any]

@router.get("/system")
async def get_system_info() -> Dict[str, Any]:
    try:
        return system_manager.get_full_system_info()
    except Exception as e:
        logger.error(f"Failed to get system info: {e}")
        raise HTTPException(status_code=500, detail="Failed to get system information")

@router.get("/system/summary")
async def get_system_summary() -> Dict[str, Any]:
    """Get system summary."""
    try:
        return system_manager.get_system_summary()
    except Exception as e:
        logger.error(f"Failed to get system summary: {e}")
        raise HTTPException(status_code=500, detail="Failed to get system summary")

@router.get("/cpu")
async def get_cpu_info() -> Dict[str, Any]:
    """Get CPU information."""
    try:
        return system_manager.get_cpu_details()
    except Exception as e:
        logger.error(f"Failed to get CPU info: {e}")
        raise HTTPException(status_code=500, detail="Failed to get CPU information")

@router.get("/memory")
async def get_memory_info() -> Dict[str, Any]:
    try:
        return system_manager.get_memory_details()
    except Exception as e:
        logger.error(f"Failed to get memory info: {e}")
        raise HTTPException(status_code=500, detail="Failed to get memory information")

@router.get("/gpu")
async def get_gpu_info() -> Dict[str, Any]:
    try:
        hardware = system_manager.get_hardware_details()
        gpus = hardware.get('gpus', [])
        
        return {
            'gpus': gpus,
            'count': len(gpus),
        }
    except Exception as e:
        logger.error(f"Failed to get GPU info: {e}")
        raise HTTPException(status_code=500, detail="Failed to get GPU information")

@router.get("/hardware")
async def get_hardware_info() -> Dict[str, Any]:
    try:
        return system_manager.get_hardware_details()
    except Exception as e:
        logger.error(f"Failed to get hardware info: {e}")
        raise HTTPException(status_code=500, detail="Failed to get hardware information")

@router.get("/metrics/current")
async def get_current_metrics() -> Dict[str, Any]:
    try:
        return await system_monitor.collect_metrics()
    except Exception as e:
        logger.error(f"Failed to get current metrics: {e}")
        raise HTTPException(status_code=500, detail="Failed to get current metrics")