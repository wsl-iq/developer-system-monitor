#!/usr/bin/env python3
# -*- coding: utf-8 -*-
# Copyright (c) 2026, lnc.
# Developer : Mohammed Al-Baqer

import logging
from typing import Any, Dict, List, Optional

from fastapi import APIRouter, HTTPException 
# Uncomment this line if FastAPI is installed
# from fastapi import APIRouter, HTTPException  # type: ignore[import-not-found]

from ..core.system_manager import SystemManager

logger = logging.getLogger(__name__)

router = APIRouter(prefix="/api/disks", tags=["disks"])

# Initialize system manager
system_manager = SystemManager()


@router.get("")
async def get_disks() -> List[Dict[str, Any]]:
    # Get disk information.# 
    try:
        return system_manager.get_disk_details()
    except Exception as e:
        logger.error(f"Failed to get disks: {e}")
        raise HTTPException(status_code=500, detail="Failed to get disk information")


@router.get("/usage")
async def get_disk_usage() -> List[Dict[str, Any]]:
    # Get disk usage information.
    try:
        disks = system_manager.get_disk_details()
        
        usage_data = []
        for disk in disks:
            usage_data.append({
                'drive_letter': disk.get('drive_letter', ''),
                'volume_name': disk.get('volume_name', ''),
                'total_bytes': disk.get('total_bytes', 0),
                'free_bytes': disk.get('free_bytes', 0),
                'used_bytes': disk.get('used_bytes', 0),
                'usage_percent': disk.get('usage_percent', 0),
                'file_system': disk.get('file_system', ''),
                'drive_type': disk.get('drive_type', ''),
            })
        
        return usage_data
    except Exception as e:
        logger.error(f"Failed to get disk usage: {e}")
        raise HTTPException(status_code=500, detail="Failed to get disk usage")


@router.get("/{drive_letter}")
async def get_disk_info(drive_letter: str) -> Dict[str, Any]:
    # Get information for a specific disk.
    try:
        disks = system_manager.get_disk_details()
        
        for disk in disks:
            if disk.get('drive_letter', '').upper() == drive_letter.upper():
                return disk
        
        raise HTTPException(status_code=404, detail=f"Drive {drive_letter} not found")
    except HTTPException:
        raise
    except Exception as e:
        logger.error(f"Failed to get disk {drive_letter}: {e}")
        raise HTTPException(status_code=500, detail="Failed to get disk information")