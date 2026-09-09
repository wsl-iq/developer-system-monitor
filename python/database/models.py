#!/usr/bin/env python3
# -*- coding: utf-8 -*-
# Copyright (c) 2026, lnc.
# Developer : Mohammed Al-Baqer

from dataclasses import dataclass
from datetime import datetime
from typing import Optional

@dataclass
class PerformanceMetric:
    timestamp: str
    cpu_usage: float
    memory_usage: float
    gpu_usage: Optional[float]
    disk_read_bytes: int
    disk_write_bytes: int
    network_received_bytes: int
    network_sent_bytes: int
    
    @classmethod
    def from_dict(cls, data: dict) -> 'PerformanceMetric':
        return cls(**data)

@dataclass
class AlertEvent:
    timestamp: str
    alert_type: str
    severity: str
    message: str
    value: float
    threshold: float
    
    @classmethod
    def from_dict(cls, data: dict) -> 'AlertEvent':
        return cls(**data)

@dataclass
class SystemSnapshot:
    timestamp: str
    data: str
    
    @classmethod
    def from_dict(cls, data: dict) -> 'SystemSnapshot':
        return cls(**data)