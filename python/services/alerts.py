#!/usr/bin/env python3
# -*- coding: utf-8 -*-
# Copyright (c) 2026, lnc.
# Developer : Mohammed Al-Baqer

import asyncio
import logging
from datetime import datetime
from typing import Any, Dict, List, Optional, Callable
from ..database.database import Database

logger = logging.getLogger(__name__)

class AlertService:
    def __init__(self, database: Optional[Database] = None):
        self.database = database or Database()
        self._alerts: List[Dict[str, Any]] = []
        self._callbacks: List[Callable] = []
        self._cooldowns: Dict[str, float] = {}
        self.cooldown_seconds = 60  # Default cooldown period
        
        # Initialize default alerts
        self._initialize_default_alerts()
    
    def _initialize_default_alerts(self):
        self._alerts = [
            {
                'id': 'cpu_high',
                'name': 'High CPU Usage',
                'type': 'cpu',
                'condition': 'above',
                'threshold': 90,
                'severity': 'warning',
                'enabled': True,
            },
            {
                'id': 'memory_high',
                'name': 'High Memory Usage',
                'type': 'memory',
                'condition': 'above',
                'threshold': 90,
                'severity': 'warning',
                'enabled': True,
            },
            {
                'id': 'gpu_high',
                'name': 'High GPU Usage',
                'type': 'gpu',
                'condition': 'above',
                'threshold': 95,
                'severity': 'warning',
                'enabled': True,
            },
            {
                'id': 'disk_low',
                'name': 'Low Disk Space',
                'type': 'disk',
                'condition': 'below',
                'threshold': 10,
                'severity': 'critical',
                'enabled': True,
            },
        ]
    
    async def check_alerts(self, metrics: Dict[str, Any]):
        import time
        current_time = time.time()
        
        for alert in self._alerts:
            if not alert['enabled']:
                continue
            
            # Check cooldown
            if alert['id'] in self._cooldowns:
                if current_time - self._cooldowns[alert['id']] < self.cooldown_seconds:
                    continue
            
            # Check condition
            value = self._get_metric_value(metrics, alert['type'])
            
            if value is not None:
                triggered = False
                
                if alert['condition'] == 'above':
                    triggered = value > alert['threshold']
                elif alert['condition'] == 'below':
                    triggered = value < alert['threshold']
                
                if triggered:
                    await self._trigger_alert(alert, value)
                    self._cooldowns[alert['id']] = current_time
    
    def _get_metric_value(self, metrics: Dict[str, Any], metric_type: str) -> Optional[float]:        
        if metric_type == 'cpu':
            return metrics.get('cpu', {}).get('usage_percent')
        elif metric_type == 'memory':
            return metrics.get('memory', {}).get('usage_percent')
        elif metric_type == 'gpu':
            return metrics.get('gpu', {}).get('usage_percent')
        elif metric_type == 'disk':
            disks = metrics.get('disks', [])
            if disks:
                # Check if any disk is below threshold (free space)
                min_free_percent = min(
                    (d.get('free_bytes', 0) / d.get('total_bytes', 1) * 100 
                     if d.get('total_bytes', 0) > 0 else 100)
                    for d in disks
                )
                return min_free_percent
        
        return None
    
    async def _trigger_alert(self, alert: Dict[str, Any], value: float):
        message = f"{alert['name']}: {value:.2f}% (threshold: {alert['threshold']}%)"
        logger.warning(f"Alert triggered: {message}")
        
        # Save to database
        if self.database:
            await self.database.insert_alert_event(
                alert_type=alert['type'],
                severity=alert['severity'],
                message=message,
                value=value,
                threshold=alert['threshold'],
            )
        
        # Notify callbacks
        alert_data = {
            'timestamp': datetime.now().isoformat(),
            'alert_type': alert['type'],
            'severity': alert['severity'],
            'message': message,
            'value': value,
            'threshold': alert['threshold'],
        }
        
        for callback in self._callbacks:
            try:
                if asyncio.iscoroutinefunction(callback):
                    await callback(alert_data)
                else:
                    callback(alert_data)
            except Exception as e:
                logger.error(f"Alert callback error: {e}")
    
    def add_alert(self, alert: Dict[str, Any]):
        self._alerts.append(alert)
    
    def remove_alert(self, alert_id: str):
        self._alerts = [a for a in self._alerts if a['id'] != alert_id]
    
    def update_alert(self, alert_id: str, updates: Dict[str, Any]):
        for alert in self._alerts:
            if alert['id'] == alert_id:
                alert.update(updates)
                break
    
    def get_alerts(self) -> List[Dict[str, Any]]:
        return self._alerts.copy()
    
    def add_callback(self, callback: Callable):
        if callback not in self._callbacks:
            self._callbacks.append(callback)
    
    def remove_callback(self, callback: Callable):
        if callback in self._callbacks:
            self._callbacks.remove(callback)