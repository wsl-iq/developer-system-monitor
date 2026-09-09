#!/usr/bin/env python3
# -*- coding: utf-8 -*-
# Copyright (c) 2026, lnc.
# Developer : Mohammed Al-Baqer

import aiosqlite
import json
import logging
import os
from datetime import datetime, timedelta
from pathlib import Path
from typing import Any, Dict, List, Optional

logger = logging.getLogger(__name__)

class Database:
    def __init__(self, db_path: Optional[str] = None):
        self.db_path = db_path or self._get_default_db_path()
        self._connection: Optional[aiosqlite.Connection] = None
    
    def _get_default_db_path(self) -> str:
        return str(Path(__file__).parent.parent.parent / 'data' / 'monitor.db')
    
    async def initialize(self):
        os.makedirs(os.path.dirname(self.db_path), exist_ok=True)
        self._connection = await aiosqlite.connect(self.db_path)
        self._connection.row_factory = aiosqlite.Row
        
        await self._create_tables()
        logger.info(f"Database initialized at {self.db_path}")
    
    async def _create_tables(self):
        # Performance metrics table
        await self._connection.execute("""
            CREATE TABLE IF NOT EXISTS performance_metrics (
                id INTEGER PRIMARY KEY AUTOINCREMENT,
                timestamp TEXT NOT NULL,
                cpu_usage REAL,
                memory_usage REAL,
                gpu_usage REAL,
                disk_read_bytes INTEGER,
                disk_write_bytes INTEGER,
                network_received_bytes INTEGER,
                network_sent_bytes INTEGER
            )
        """)
        
        # Alert events table
        await self._connection.execute("""
            CREATE TABLE IF NOT EXISTS alert_events (
                id INTEGER PRIMARY KEY AUTOINCREMENT,
                timestamp TEXT NOT NULL,
                alert_type TEXT NOT NULL,
                severity TEXT NOT NULL,
                message TEXT NOT NULL,
                value REAL,
                threshold REAL
            )
        """)
        
        # System snapshots table
        await self._connection.execute("""
            CREATE TABLE IF NOT EXISTS system_snapshots (
                id INTEGER PRIMARY KEY AUTOINCREMENT,
                timestamp TEXT NOT NULL,
                data TEXT NOT NULL
            )
        """)
        
        # Create indexes
        await self._connection.execute("""
            CREATE INDEX IF NOT EXISTS idx_performance_timestamp 
            ON performance_metrics(timestamp)
        """)
        
        await self._connection.execute("""
            CREATE INDEX IF NOT EXISTS idx_alerts_timestamp 
            ON alert_events(timestamp)
        """)
        
        await self._connection.commit()
    
    async def close(self):
        if self._connection:
            await self._connection.close()
            self._connection = None
    
    async def insert_performance_metric(self, metrics: Dict[str, Any]):
        if not self._connection:
            await self.initialize()
        timestamp = metrics.get('timestamp', datetime.now().isoformat())
        cpu = metrics.get('cpu', {})
        memory = metrics.get('memory', {})
        
        await self._connection.execute("""
            INSERT INTO performance_metrics 
            (timestamp, cpu_usage, memory_usage, gpu_usage, 
             disk_read_bytes, disk_write_bytes, 
             network_received_bytes, network_sent_bytes)
            VALUES (?, ?, ?, ?, ?, ?, ?, ?)
        """, (
            timestamp,
            cpu.get('usage_percent', 0),
            memory.get('usage_percent', 0),
            metrics.get('gpu', {}).get('usage_percent', 0),
            sum(d.get('read_bytes_per_sec', 0) for d in metrics.get('disks', [])),
            sum(d.get('write_bytes_per_sec', 0) for d in metrics.get('disks', [])),
            sum(n.get('bytes_received_per_sec', 0) for n in metrics.get('network', [])),
            sum(n.get('bytes_sent_per_sec', 0) for n in metrics.get('network', [])),
        ))
        
        await self._connection.commit()
    
    async def insert_alert_event(self, alert_type: str, severity: str, message: str, value: float, threshold: float):
        if not self._connection:
            await self.initialize()
        
        timestamp = datetime.now().isoformat()
        
        await self._connection.execute("""
            INSERT INTO alert_events 
            (timestamp, alert_type, severity, message, value, threshold)
            VALUES (?, ?, ?, ?, ?, ?)
        """, (timestamp, alert_type, severity, message, value, threshold))
        
        await self._connection.commit()
    
    async def get_performance_history(self, hours: int = 1) -> List[Dict[str, Any]]:

        if not self._connection:
            await self.initialize()
        
        since = datetime.now() - timedelta(hours=hours)
        
        cursor = await self._connection.execute("""
            SELECT * FROM performance_metrics 
            WHERE timestamp >= ?
            ORDER BY timestamp ASC
        """, (since.isoformat(),))
        
        rows = await cursor.fetchall()
        return [dict(row) for row in rows]
    
    async def get_alert_history(self, hours: int = 24) -> List[Dict[str, Any]]:
        if not self._connection:
            await self.initialize()
        since = datetime.now() - timedelta(hours=hours)

        cursor = await self._connection.execute("""
            SELECT * FROM alert_events 
            WHERE timestamp >= ?
            ORDER BY timestamp DESC
        """, (since.isoformat(),))
        
        rows = await cursor.fetchall()
        return [dict(row) for row in rows]
    
    async def cleanup_old_data(self, retention_hours: int = 24):
        if not self._connection:
            await self.initialize()
        
        cutoff = datetime.now() - timedelta(hours=retention_hours)
        
        await self._connection.execute("""
            DELETE FROM performance_metrics 
            WHERE timestamp < ?
        """, (cutoff.isoformat(),))
        
        await self._connection.execute("""
            DELETE FROM alert_events 
            WHERE timestamp < ?
        """, (cutoff.isoformat(),))
        
        await self._connection.commit()
        logger.info(f"Cleaned up data older than {retention_hours} hours")