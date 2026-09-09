#!/usr/bin/env python3
# -*- coding: utf-8 -*-
# Copyright (c) 2026, lnc.
# Developer : Mohammed Al-Baqer

import json
import logging
from typing import Any, Dict, List, Optional
from .native import NativeInterface, get_native_interface

logger = logging.getLogger(__name__)


class SystemManager:
    
    def __init__(self, native: Optional[NativeInterface] = None):
        self.native = native or get_native_interface()
    
    def get_full_system_info(self) -> Dict[str, Any]:
        info = {
            'system': self.native.get_system_info(),
            'cpu': self.native.get_cpu_info(),
            'memory': self.native.get_memory_info(),
            'disks': self.native.get_disk_info(),
            'network': self.native.get_network_info(),
            'hardware': self.native.get_hardware_info(),
        }
        
        return info
    
    def get_system_summary(self) -> Dict[str, Any]:
        summary = {}
        system_info = self.native.get_system_info()
        if system_info:
            summary['hostname'] = system_info.get('hostname', 'Unknown')
            summary['os_name'] = system_info.get('os_name', 'Unknown')
            summary['os_version'] = system_info.get('os_version', 'Unknown')
            summary['uptime_seconds'] = system_info.get('uptime_seconds', 0)
            summary['architecture'] = system_info.get('architecture', 'Unknown')
        
        cpu_info = self.native.get_cpu_info()
        if cpu_info:
            summary['cpu_model'] = cpu_info.get('model', 'Unknown')
            summary['cpu_manufacturer'] = cpu_info.get('manufacturer', 'Unknown')
            summary['cpu_cores'] = cpu_info.get('physical_cores', 0)
            summary['cpu_threads'] = cpu_info.get('logical_processors', 0)
            summary['cpu_frequency_mhz'] = cpu_info.get('current_frequency_mhz', 0)
        
        memory_info = self.native.get_memory_info()
        if memory_info:
            summary['memory_total_bytes'] = memory_info.get('total_physical_bytes', 0)
            summary['memory_available_bytes'] = memory_info.get('available_physical_bytes', 0)
            summary['memory_usage_percent'] = memory_info.get('usage_percent', 0)
        
        return summary
    
    def get_cpu_details(self) -> Dict[str, Any]:
        return self.native.get_cpu_info() or {}
    
    def get_memory_details(self) -> Dict[str, Any]:
        return self.native.get_memory_info() or {}
    
    def get_disk_details(self) -> List[Dict[str, Any]]:
        return self.native.get_disk_info() or []
    
    def get_network_details(self) -> List[Dict[str, Any]]:
        return self.native.get_network_info() or []
    
    def get_hardware_details(self) -> Dict[str, Any]:
        return self.native.get_hardware_info() or {}
    
    def export_system_info(self, file_path: str, format: str = 'json') -> bool:
        try:
            info = self.get_full_system_info()
            
            if format.lower() == 'json':
                with open(file_path, 'w') as f:
                    json.dump(info, f, indent=2)
                return True
            elif format.lower() == 'html':
                # Generate HTML report
                html_content = self._generate_html_report(info)
                with open(file_path, 'w') as f:
                    f.write(html_content)
                return True
            else:
                logger.error(f"Unsupported export format: {format}")
                return False
                
        except Exception as e:
            logger.error(f"Failed to export system info: {e}")
            return False
    
    def _generate_html_report(self, info: Dict[str, Any]) -> str:        
        html = """<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <title>System Information Report</title>
    <style>
        body { font-family: Arial, sans-serif; margin: 20px; }
        h1 { color: #333; }
        table { border-collapse: collapse; width: 100%; margin: 10px 0; }
        th, td { border: 1px solid #ddd; padding: 8px; text-align: left; }
        th { background-color: #f2f2f2; }
        tr:nth-child(even) { background-color: #f9f9f9; }
    </style>
</head>
<body>
    <h1>System Information Report</h1>
"""
        # Add system information
        if info.get('system'):
            html += "<h2>System</h2><table>"
            for key, value in info['system'].items():
                html += f"<tr><th>{key}</th><td>{value}</td></tr>"
            html += "</table>"
        
        # Add CPU information
        if info.get('cpu'):
            html += "<h2>CPU</h2><table>"
            for key, value in info['cpu'].items():
                html += f"<tr><th>{key}</th><td>{value}</td></tr>"
            html += "</table>"
        
        # Add memory information
        if info.get('memory'):
            html += "<h2>Memory</h2><table>"
            for key, value in info['memory'].items():
                if key != 'modules':  # Skip modules for now
                    html += f"<tr><th>{key}</th><td>{value}</td></tr>"
            html += "</table>"
        html += "</body></html>"
        return html