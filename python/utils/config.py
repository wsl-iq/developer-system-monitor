#!/usr/bin/env python3
# -*- coding: utf-8 -*-
# Copyright (c) 2026, lnc.
# Developer : Mohammed Al-Baqer

import json
import logging
import os
from pathlib import Path
from typing import Any, Dict, Optional

logger = logging.getLogger(__name__)

class Config:    
    DEFAULT_CONFIG = {
        'monitoring': {
            'interval_seconds': 2,
            'history_retention_hours': 24,
            'enable_gpu_monitoring': True,
            'enable_temperature_monitoring': True,
        },
        'alerts': {
            'cpu_threshold_percent': 90,
            'memory_threshold_percent': 90,
            'gpu_threshold_percent': 95,
            'disk_threshold_percent': 90,
            'enable_alerts': True,
            'alert_cooldown_seconds': 60,
        },
        'server': {
            'host': '127.0.0.1',
            'port': 8000,
            'enable_cors': False,
        },
        'logging': {
            'level': 'INFO',
            'log_directory': 'logs',
        },
        'ui': {
            'theme': 'dark',
            'language': 'en',
            'auto_refresh': True,
            'refresh_interval_seconds': 2,
        },
    }
    
    def __init__(self, config_path: Optional[str] = None):
        self.config_path = config_path or self._get_default_config_path()
        self.config = self._load_config()
    
    def _get_default_config_path(self) -> str:
        return str(Path(__file__).parent.parent.parent / 'data' / 'config.json')
    
    def _load_config(self) -> Dict[str, Any]:
        config = self.DEFAULT_CONFIG.copy()
        if os.path.exists(self.config_path):
            try:
                with open(self.config_path, 'r') as f:
                    user_config = json.load(f)
                config = self._merge_configs(config, user_config)
                logger.info(f"Loaded configuration from {self.config_path}")
            except Exception as e:
                logger.error(f"Failed to load config: {e}")
                logger.info("Using default configuration")
        else:
            logger.info("No configuration file found, using defaults")
            self._save_config(config)
        
        return config
    
    def _merge_configs(self, default: Dict[str, Any], user: Dict[str, Any]) -> Dict[str, Any]:
        merged = default.copy()
        for key, value in user.items():
            if (key in merged and isinstance(merged[key], dict) 
                and isinstance(value, dict)):
                merged[key] = self._merge_configs(merged[key], value)
            else:
                merged[key] = value
        return merged
    
    def _save_config(self, config: Dict[str, Any]) -> None:
        try:
            os.makedirs(os.path.dirname(self.config_path), exist_ok=True)
            with open(self.config_path, 'w') as f:
                json.dump(config, f, indent=2)
            logger.info(f"Saved configuration to {self.config_path}")
        except Exception as e:
            logger.error(f"Failed to save config: {e}")
    
    def get(self, section: str, key: str, default: Any = None) -> Any:
        try:
            return self.config.get(section, {}).get(key, default)
        except AttributeError:
            return default
    
    def set(self, section: str, key: str, value: Any) -> None:
        if section not in self.config:
            self.config[section] = {}
        
        self.config[section][key] = value
        self._save_config(self.config)
    
    def get_all(self) -> Dict[str, Any]:
        return self.config.copy()
    
    def update(self, updates: Dict[str, Any]) -> None:
        self.config = self._merge_configs(self.config, updates)
        self._save_config(self.config)
    
    def reset(self) -> None:
        self.config = self.DEFAULT_CONFIG.copy()
        self._save_config(self.config)
        logger.info("Configuration reset to defaults")