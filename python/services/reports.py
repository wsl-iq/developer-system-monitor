#!/usr/bin/env python3
# -*- coding: utf-8 -*-
# Copyright (c) 2026, lnc.
# Developer : Mohammed Al-Baqer

import json
import logging
import os
from datetime import datetime
from pathlib import Path
from typing import Any, Dict, List, Optional
from ..core.system_manager import SystemManager

logger = logging.getLogger(__name__)

class ReportGenerator:
    def __init__(self, system_manager: Optional[SystemManager] = None):
        self.system_manager = system_manager or SystemManager()
    
    def generate_system_report(self, output_path: Optional[str] = None, format: str = 'json') -> str:
        if output_path is None:
            output_dir = Path(__file__).parent.parent.parent / 'data' / 'reports'
            os.makedirs(output_dir, exist_ok=True)
            
            timestamp = datetime.now().strftime('%Y%m%d_%H%M%S')
            filename = f'SystemReport_{timestamp}.{format}'
            output_path = str(output_dir / filename)
        
        if format.lower() == 'json':
            return self._generate_json_report(output_path)
        
        elif format.lower() == 'html':
            return self._generate_html_report(output_path)
        
        else:
            logger.error(f"Unsupported report format: {format}")
            return ""
    
    def _generate_json_report(self, output_path: str) -> str:
        info = self.system_manager.get_full_system_info()
        
        with open(output_path, 'w') as f:
            json.dump(info, f, indent=2, default=str)
        
        logger.info(f"JSON report generated: {output_path}")
        return output_path
    
    def _generate_html_report(self, output_path: str) -> str:
        info = self.system_manager.get_full_system_info()
        html_content = self._build_html_report(info)
        
        with open(output_path, 'w', encoding='utf-8') as f:
            f.write(html_content)
        
        logger.info(f"HTML report generated: {output_path}")
        return output_path
    
    def _build_html_report(self, info: Dict[str, Any]) -> str:        
        html = """<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>System Report</title>
    <style>
        body {
            font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', Roboto, sans-serif;
            margin: 20px;
            background-color: #f5f5f5;
            color: #333;
        }
        h1 {
            color: #2c3e50;
            border-bottom: 2px solid #3498db;
            padding-bottom: 10px;
        }
        h2 {
            color: #34495e;
            margin-top: 30px;
        }
        table {
            border-collapse: collapse;
            width: 100%;
            margin: 10px 0;
            background-color: white;
            box-shadow: 0 1px 3px rgba(0,0,0,0.1);
        }
        th, td {
            border: 1px solid #ddd;
            padding: 12px;
            text-align: left;
        }
        th {
            background-color: #3498db;
            color: white;
            font-weight: 600;
        }
        tr:nth-child(even) {
            background-color: #f9f9f9;
        }
        .section {
            background-color: white;
            padding: 20px;
            margin: 20px 0;
            border-radius: 5px;
            box-shadow: 0 2px 5px rgba(0,0,0,0.1);
        }
        .timestamp {
            color: #7f8c8d;
            font-size: 0.9em;
        }
    </style>
</head>
<body>
    <h1>System Report</h1>
    <p class="timestamp">Generated: {timestamp}</p>
"""
        
        html = html.format(timestamp=datetime.now().strftime('%Y-%m-%d %H:%M:%S'))
        
        # System information
        if info.get('system'):
            html += '<div class="section"><h2>System Information</h2><table>'
            for key, value in info['system'].items():
                html += f'<tr><th>{key.replace("_", " ").title()}</th><td>{value}</td></tr>'
            html += '</table></div>'
        
        # CPU information
        if info.get('cpu'):
            html += '<div class="section"><h2>CPU</h2><table>'
            for key, value in info['cpu'].items():
                html += f'<tr><th>{key.replace("_", " ").title()}</th><td>{value}</td></tr>'
            html += '</table></div>'
        
        # Memory information
        if info.get('memory'):
            html += '<div class="section"><h2>Memory</h2><table>'
            memory = info['memory']
            total_gb = memory.get('total_physical_bytes', 0) / (1024**3)
            used_gb = memory.get('used_physical_bytes', 0) / (1024**3)
            html += f'<tr><th>Total Memory</th><td>{total_gb:.2f} GB</td></tr>'
            html += f'<tr><th>Used Memory</th><td>{used_gb:.2f} GB</td></tr>'
            html += f'<tr><th>Usage</th><td>{memory.get("usage_percent", 0):.2f}%</td></tr>'
            html += '</table></div>'
        
        # Disk information
        if info.get('disks'):
            html += '<div class="section"><h2>Disks</h2><table>'
            html += '<tr><th>Drive</th><th>Total</th><th>Free</th><th>Usage</th></tr>'
            for disk in info['disks']:
                total_gb = disk.get('total_bytes', 0) / (1024**3)
                free_gb = disk.get('free_bytes', 0) / (1024**3)
                html += f'<tr><td>{disk.get("drive_letter", "Unknown")}</td>'
                html += f'<td>{total_gb:.2f} GB</td><td>{free_gb:.2f} GB</td>'
                html += f'<td>{disk.get("usage_percent", 0):.2f}%</td></tr>'
            html += '</table></div>'
        
        html += '</body></html>'
        
        return html
    
    def generate_performance_report(self, performance_data: List[Dict[str, Any]], output_path: Optional[str] = None) -> str:
        if output_path is None:
            output_dir = Path(__file__).parent.parent.parent / 'data' / 'reports'
            os.makedirs(output_dir, exist_ok=True)
            
            timestamp = datetime.now().strftime('%Y%m%d_%H%M%S')
            filename = f'PerformanceReport_{timestamp}.json'
            output_path = str(output_dir / filename)
        
        report = {
            'generated_at': datetime.now().isoformat(),
            'data_points': len(performance_data),
            'metrics': performance_data,
        }
        
        with open(output_path, 'w') as f:
            json.dump(report, f, indent=2, default=str)
        
        logger.info(f"Performance report generated: {output_path}")
        return output_path