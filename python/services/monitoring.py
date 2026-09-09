#!/usr/bin/env python3
# -*- coding: utf-8 -*-
# Copyright (c) 2026, lnc.
# Developer : Mohammed Al-Baqer

import asyncio
import logging
from typing import Any, Dict, Optional
from ..core.monitor import SystemMonitor
from ..database.database import Database

logger = logging.getLogger(__name__)

class MonitoringService:    
    def __init__(self, monitor: Optional[SystemMonitor] = None, database: Optional[Database] = None):

        self.monitor = monitor or SystemMonitor()
        self.database = database or Database()
        self._running = False
        self._task: Optional[asyncio.Task] = None
        self._save_interval = 5  # Save to database every 5 seconds
        self._last_save_time = 0
    
    async def start(self):
        if self._running:
            return
        
        await self.database.initialize()
        self._running = True
        await self.monitor.start() # Start monitor
        self._task = asyncio.create_task(self._persistence_loop()) # Start data persistence task
        
        logger.info("Monitoring service started")
    
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
        
        await self.monitor.stop()
        await self.database.close()
        
        logger.info("Monitoring service stopped")
    
    async def _persistence_loop(self):
        import time
        
        while self._running:
            try:
                current_time = time.time()
                
                if current_time - self._last_save_time >= self._save_interval:
                    metrics = self.monitor.get_latest_metrics()
                    
                    if metrics:
                        await self.database.insert_performance_metric(metrics)
                        self._last_save_time = current_time
                
                await asyncio.sleep(1)
            except asyncio.CancelledError:
                break
            except Exception as e:
                logger.error(f"Persistence loop error: {e}")
                await asyncio.sleep(5)
    
    def get_latest_metrics(self) -> Dict[str, Any]:
        return self.monitor.get_latest_metrics()
    
    async def get_history(self, hours: int = 1):
        return await self.database.get_performance_history(hours)