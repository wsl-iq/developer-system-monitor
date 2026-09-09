#!/usr/bin/env python3
# -*- coding: utf-8 -*-
# Copyright (c) 2026, lnc.
# Developer : Mohammed Al-Baqer

import logging
from typing import Any, Dict, List, Optional

from fastapi import APIRouter, HTTPException

from ..core.system_manager import SystemManager

logger = logging.getLogger(__name__)

router = APIRouter(prefix="/api/network", tags=["network"])

# Initialize system manager
system_manager = SystemManager()


@router.get("")
async def get_network_adapters() -> List[Dict[str, Any]]:
    # Get network adapter information.
    try:
        return system_manager.get_network_details()
    except Exception as e:
        logger.error(f"Failed to get network adapters: {e}")
        raise HTTPException(status_code=500, detail="Failed to get network information")

@router.get("/stats")
async def get_network_stats() -> Dict[str, Any]:
    # Get network statistics.
    try:
        adapters = system_manager.get_network_details()
        
        total_received = sum(a.get('bytes_received', 0) for a in adapters)
        total_sent = sum(a.get('bytes_sent', 0) for a in adapters)
        
        return {
            'total_adapters': len(adapters),
            'total_bytes_received': total_received,
            'total_bytes_sent': total_sent,
            'active_adapters': sum(1 for a in adapters if a.get('status') == 'Up'),
        }
    except Exception as e:
        logger.error(f"Failed to get network stats: {e}")
        raise HTTPException(status_code=500, detail="Failed to get network statistics")