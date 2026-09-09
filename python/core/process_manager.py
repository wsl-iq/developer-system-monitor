#!/usr/bin/env python3
# -*- coding: utf-8 -*-
# Copyright (c) 2026, lnc.
# Developer : Mohammed Al-Baqer

import logging
from typing import Any, Dict, List, Optional
from .native import NativeInterface, get_native_interface
logger = logging.getLogger(__name__)

class ProcessManager:    
    def __init__(self, native: Optional[NativeInterface] = None):
        self.native = native or get_native_interface()
    
    def get_processes(self) -> List[Dict[str, Any]]:
        return self.native.get_process_list() or []
    
    def get_process(self, pid: int) -> Optional[Dict[str, Any]]:
        processes = self.get_processes()
        for process in processes:
            if process.get('pid') == pid:
                return process
        return None
    
    def terminate_process(self, pid: int) -> bool:
        # Check if process exists
        process = self.get_process(pid)
        if not process:
            logger.warning(f"Process {pid} not found")
            return False
        
        logger.info(f"Terminating process {pid} ({process.get('name', 'Unknown')})")
        return self.native.terminate_process(pid)
    
    def suspend_process(self, pid: int) -> bool:
        process = self.get_process(pid)
        if not process:
            logger.warning(f"Process {pid} not found")
            return False
        
        logger.info(f"Suspending process {pid} ({process.get('name', 'Unknown')})")
        return self.native.suspend_process(pid)
    
    def resume_process(self, pid: int) -> bool:
        process = self.get_process(pid)
        if not process:
            logger.warning(f"Process {pid} not found")
            return False
        
        logger.info(f"Resuming process {pid} ({process.get('name', 'Unknown')})")
        return self.native.resume_process(pid)
    
    def search_processes(self, query: str) -> List[Dict[str, Any]]:
        query = query.lower()
        processes = self.get_processes()
        
        results = []
        for process in processes:
            name = process.get('name', '').lower()
            pid = str(process.get('pid', ''))
            
            if query in name or query in pid:
                results.append(process)
        
        return results
    
    def sort_processes(self, processes: List[Dict[str, Any]], key: str, reverse: bool = False) -> List[Dict[str, Any]]:
        try:
            return sorted(processes, key=lambda x: x.get(key, 0), reverse=reverse)
        except (TypeError, KeyError):
            return processes
    
    def get_process_count(self) -> int:
        return len(self.get_processes())
    
    def get_top_processes(self, count: int = 10, sort_by: str = 'memory_usage_bytes') -> List[Dict[str, Any]]:
        processes = self.get_processes()
        sorted_processes = self.sort_processes(processes, sort_by, reverse=True)
        return sorted_processes[:count]