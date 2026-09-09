#!/usr/bin/env python3
# -*- coding: utf-8 -*-
# Copyright (c) 2026, lnc.
# Developer : Mohammed Al-Baqer

import logging
from typing import Any, Dict, List, Optional

from fastapi import APIRouter, HTTPException, Query
from pydantic import BaseModel, Field

from ..core.process_manager import ProcessManager

logger = logging.getLogger(__name__)

router = APIRouter(prefix="/api/processes", tags=["processes"])

# Initialize process manager
process_manager = ProcessManager()

class TerminateRequest(BaseModel):
    pid: int = Field(..., description="Process ID to terminate")

class SuspendRequest(BaseModel):
    pid: int = Field(..., description="Process ID to suspend")

class ResumeRequest(BaseModel):
    # Process resumption request model.
    pid: int = Field(..., description="Process ID to resume")

@router.get("")
async def get_processes(
    search: Optional[str] = Query(None, description="Search query"),
    sort_by: Optional[str] = Query(None, description="Sort field"),
    sort_order: Optional[str] = Query("asc", description="Sort order (asc/desc)"),
    limit: Optional[int] = Query(None, description="Limit number of results"),
) -> List[Dict[str, Any]]:
    # Get process list.
    try:
        processes = process_manager.get_processes()
        
        # Apply search
        if search:
            processes = process_manager.search_processes(search)
        
        # Apply sorting
        if sort_by:
            reverse = sort_order.lower() == 'desc'
            processes = process_manager.sort_processes(processes, sort_by, reverse)
        
        # Apply limit
        if limit and limit > 0:
            processes = processes[:limit]
        
        return processes
    except Exception as e:
        logger.error(f"Failed to get processes: {e}")
        raise HTTPException(status_code=500, detail="Failed to get process list")

@router.get("/{pid}")
async def get_process(pid: int) -> Dict[str, Any]:
    # Get process information.
    try:
        process = process_manager.get_process(pid)
        if not process:
            raise HTTPException(status_code=404, detail=f"Process {pid} not found")
        return process
    except HTTPException:
        raise
    except Exception as e:
        logger.error(f"Failed to get process {pid}: {e}")
        raise HTTPException(status_code=500, detail="Failed to get process information")

@router.post("/{pid}/terminate")
async def terminate_process(pid: int) -> Dict[str, Any]:
    # Terminate a process.
    try:
        success = process_manager.terminate_process(pid)
        if not success:
            raise HTTPException(status_code=400, detail=f"Failed to terminate process {pid}")
        
        return {
            'success': True,
            'message': f"Process {pid} terminated successfully",
            'pid': pid,
        }
    except HTTPException:
        raise
    except Exception as e:
        logger.error(f"Failed to terminate process {pid}: {e}")
        raise HTTPException(status_code=500, detail="Failed to terminate process")

@router.post("/{pid}/suspend")
async def suspend_process(pid: int) -> Dict[str, Any]:
    # Suspend a process.
    try:
        success = process_manager.suspend_process(pid)
        if not success:
            raise HTTPException(status_code=400, detail=f"Failed to suspend process {pid}")
        
        return {
            'success': True,
            'message': f"Process {pid} suspended successfully",
            'pid': pid,
        }
    except HTTPException:
        raise
    except Exception as e:
        logger.error(f"Failed to suspend process {pid}: {e}")
        raise HTTPException(status_code=500, detail="Failed to suspend process")

@router.post("/{pid}/resume")
async def resume_process(pid: int) -> Dict[str, Any]:
    # Resume a process.
    try:
        success = process_manager.resume_process(pid)
        if not success:
            raise HTTPException(status_code=400, detail=f"Failed to resume process {pid}")
        
        return {
            'success': True,
            'message': f"Process {pid} resumed successfully",
            'pid': pid,
        }
    except HTTPException:
        raise
    except Exception as e:
        logger.error(f"Failed to resume process {pid}: {e}")
        raise HTTPException(status_code=500, detail="Failed to resume process")


@router.get("/stats/summary")
async def get_process_stats() -> Dict[str, Any]:
    # Get process statistics.
    try:
        processes = process_manager.get_processes()
        
        total_memory = sum(p.get('memory_usage_bytes', 0) for p in processes)
        total_cpu = sum(p.get('cpu_usage_percent', 0) for p in processes)
        
        return {
            'total_processes': len(processes),
            'total_memory_bytes': total_memory,
            'total_cpu_percent': total_cpu,
            'top_memory_processes': process_manager.get_top_processes(5, 'memory_usage_bytes'),
            'top_cpu_processes': process_manager.get_top_processes(5, 'cpu_usage_percent'),
        }
    except Exception as e:
        logger.error(f"Failed to get process stats: {e}")
        raise HTTPException(status_code=500, detail="Failed to get process statistics")