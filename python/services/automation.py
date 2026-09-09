#!/usr/bin/env python3
# -*- coding: utf-8 -*-
# Copyright (c) 2026, lnc.
# Developer : Mohammed Al-Baqer

import asyncio
import logging
from datetime import datetime, timedelta
from typing import Any, Dict, List, Optional, Callable

logger = logging.getLogger(__name__)

class AutomationService:
    def __init__(self):
        self._tasks: List[Dict[str, Any]] = []
        self._running_tasks: Dict[str, asyncio.Task] = {}
        self._running = False
    
    async def start(self):
        self._running = True
        logger.info("Automation service started")
    
    async def stop(self):
        self._running = False
        
        # Cancel all running tasks
        for task_id, task in self._running_tasks.items():
            task.cancel()
        
        self._running_tasks.clear()
        logger.info("Automation service stopped")
    
    def add_task(self, task_id: str, name: str, interval_seconds: int, callback: Callable, *args, **kwargs):
        
        # Add an automated task.
        # Args:
        #     task_id: Unique task identifier.
        #     name: Task name.
        #     interval_seconds: Execution interval in seconds.
        #     callback: Function to execute.
        #     *args: Positional arguments for callback.
        #     **kwargs: Keyword arguments for callback.
        
        task_info = {
            'id': task_id,
            'name': name,
            'interval': interval_seconds,
            'callback': callback,
            'args': args,
            'kwargs': kwargs,
            'enabled': True,
            'last_run': None,
            'run_count': 0,
        }
        
        self._tasks.append(task_info)
        logger.info(f"Added automation task: {name}")
    
    def remove_task(self, task_id: str):
        self._tasks = [t for t in self._tasks if t['id'] != task_id]
        
        # Cancel running task if exists
        if task_id in self._running_tasks:
            self._running_tasks[task_id].cancel()
            del self._running_tasks[task_id]
        
        logger.info(f"Removed automation task: {task_id}")
    
    def get_tasks(self) -> List[Dict[str, Any]]:
        return self._tasks.copy()
    
    async def run_task(self, task_id: str):
        task = self._find_task(task_id)
        if task and task['enabled']:
            await self._execute_task(task)
    
    async def _execute_task(self, task: Dict[str, Any]):
        try:
            callback = task['callback']
            args = task['args']
            kwargs = task['kwargs']
            
            if asyncio.iscoroutinefunction(callback):
                await callback(*args, **kwargs)
            else:
                callback(*args, **kwargs)
            
            task['last_run'] = datetime.now().isoformat()
            task['run_count'] += 1
            
            logger.debug(f"Task executed: {task['name']} (run {task['run_count']})")
        except Exception as e:
            logger.error(f"Task execution error ({task['name']}): {e}")
    
    def _find_task(self, task_id: str) -> Optional[Dict[str, Any]]:
        for task in self._tasks:
            if task['id'] == task_id:
                return task
        return None