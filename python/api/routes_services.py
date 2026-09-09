#!/usr/bin/env python3
# -*- coding: utf-8 -*-
# Copyright (c) 2026, lnc.
# Developer : Mohammed Al-Baqer

import logging
from typing import Any, Dict, List, Optional
from fastapi import APIRouter, HTTPException
from ..core.native import get_native_interface

logger = logging.getLogger(__name__)
router = APIRouter(prefix="/api/services", tags=["services"])

# Initialize native interface
native = get_native_interface()


@router.get("")
async def get_services() -> List[Dict[str, Any]]:
    try:
        return native.get_service_list() or []
    except Exception as e:
        logger.error(f"Failed to get services: {e}")
        raise HTTPException(status_code=500, detail="Failed to get services list")


@router.get("/{service_name}")
async def get_service_info(service_name: str) -> Dict[str, Any]:
    try:
        services = native.get_service_list() or []
        
        for service in services:
            if service.get('service_name') == service_name:
                return service
        
        raise HTTPException(status_code=404, detail=f"Service {service_name} not found")
    except HTTPException:
        raise
    except Exception as e:
        logger.error(f"Failed to get service {service_name}: {e}")
        raise HTTPException(status_code=500, detail="Failed to get service information")


@router.post("/{service_name}/start")
async def start_service(service_name: str) -> Dict[str, Any]:
    try:
        success = native.start_service(service_name)
        if not success:
            raise HTTPException(status_code=400, detail=f"Failed to start service {service_name}")
        
        return {
            'success': True,
            'message': f"Service {service_name} started successfully",
            'service_name': service_name,
        }
    except HTTPException:
        raise
    except Exception as e:
        logger.error(f"Failed to start service {service_name}: {e}")
        raise HTTPException(status_code=500, detail="Failed to start service")


@router.post("/{service_name}/stop")
async def stop_service(service_name: str) -> Dict[str, Any]:
    try:
        success = native.stop_service(service_name)
        if not success:
            raise HTTPException(status_code=400, detail=f"Failed to stop service {service_name}")
        
        return {
            'success': True,
            'message': f"Service {service_name} stopped successfully",
            'service_name': service_name,
        }
    except HTTPException:
        raise
    except Exception as e:
        logger.error(f"Failed to stop service {service_name}: {e}")
        raise HTTPException(status_code=500, detail="Failed to stop service")


@router.post("/{service_name}/restart")
async def restart_service(service_name: str) -> Dict[str, Any]:
    try:
        success = native.restart_service(service_name)
        if not success:
            raise HTTPException(status_code=400, detail=f"Failed to restart service {service_name}")
        
        return {
            'success': True,
            'message': f"Service {service_name} restarted successfully",
            'service_name': service_name,
        }
    except HTTPException:
        raise
    except Exception as e:
        logger.error(f"Failed to restart service {service_name}: {e}")
        raise HTTPException(status_code=500, detail="Failed to restart service")