#!/usr/bin/env python3
# -*- coding: utf-8 -*-
# Copyright (c) 2026, lnc.
# Developer : Mohammed Al-Baqer

import sys
import json
from pathlib import Path

# Add project root to Python path
project_root = Path(__file__).parent.parent
sys.path.insert(0, str(project_root))

from python.utils.logger import setup_logging
from python.core.native import NativeInterface

def test_native_interface():
    # Set up logging
    logger = setup_logging()
    logger.info("Testing native interface...")
    
    # Initialize native interface
    native = NativeInterface()
    
    if not native.is_available():
        logger.error("Native library not available")
        logger.info("Please build the C++ core first using scripts/build.bat")
        return False
    
    logger.info("Native library loaded successfully")
    
    # Test system info
    logger.info("\n System Information ")
    system_info = native.get_system_info()
    if system_info:
        logger.info(json.dumps(system_info, indent=2))
    else:
        logger.error("Failed to get system info")
    
    # Test CPU info
    logger.info("\n CPU Information ")
    cpu_info = native.get_cpu_info()
    if cpu_info:
        logger.info(f"CPU: {cpu_info.get('manufacturer', 'Unknown')} {cpu_info.get('model', 'Unknown')}")
        logger.info(f"Cores: {cpu_info.get('physical_cores', 0)}, Threads: {cpu_info.get('logical_processors', 0)}")
        logger.info(f"Frequency: {cpu_info.get('current_frequency_mhz', 0)} MHz")
        logger.info(f"Usage: {cpu_info.get('total_usage_percent', 0):.2f}%")
    else:
        logger.error("Failed to get CPU info")
    
    # Test memory info
    logger.info("\n Memory Information ")
    memory_info = native.get_memory_info()
    if memory_info:
        total_gb = memory_info.get('total_physical_bytes', 0) / (1024**3)
        used_gb = memory_info.get('used_physical_bytes', 0) / (1024**3)
        logger.info(f"Total: {total_gb:.2f} GB")
        logger.info(f"Used: {used_gb:.2f} GB")
        logger.info(f"Usage: {memory_info.get('usage_percent', 0):.2f}%")
    else:
        logger.error("Failed to get memory info")
    
    # Test process list
    logger.info("\n Process List (first 5) ")
    processes = native.get_process_list()
    if processes:
        for proc in processes[:5]:
            logger.info(f"PID: {proc.get('pid', 0)}, Name: {proc.get('name', 'Unknown')}")
    else:
        logger.error("Failed to get process list")
    
    # Test disk info
    logger.info("\n Disk Information ")
    disks = native.get_disk_info()
    if disks:
        for disk in disks:
            total_gb = disk.get('total_bytes', 0) / (1024**3)
            free_gb = disk.get('free_bytes', 0) / (1024**3)
            logger.info(f"{disk.get('drive_letter', 'Unknown')}: {free_gb:.2f} GB free / {total_gb:.2f} GB total")
    else:
        logger.error("Failed to get disk info")
    
    # Test network info
    logger.info("\n Network Information ")
    network = native.get_network_info()
    if network:
        for adapter in network:
            logger.info(f"Adapter: {adapter.get('description', 'Unknown')}")
            logger.info(f"  MAC: {adapter.get('mac_address', 'Unknown')}")
    else:
        logger.error("Failed to get network info")
    
    # Test hardware info
    logger.info("\n Hardware Information ")
    hardware = native.get_hardware_info()
    if hardware:
        if 'cpu' in hardware:
            logger.info(f"CPU: {hardware['cpu'].get('model', 'Unknown')}")
        if 'gpus' in hardware and hardware['gpus']:
            logger.info(f"GPU: {hardware['gpus'][0].get('name', 'Unknown')}")
    else:
        logger.error("Failed to get hardware info")
    
    logger.info("\nNative interface testing complete")
    return True

if __name__ == "__main__":
    success = test_native_interface()
    sys.exit(0 if success else 1)