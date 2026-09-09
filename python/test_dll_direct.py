#!/usr/bin/env python3
# -*- coding: utf-8 -*-
# Copyright (c) 2026, lnc.
# Developer : Mohammed Al-Baqer

# Direct DLL test without the full wrapper.

import ctypes
import os
import sys
import traceback

# Add MinGW to PATH
os.environ['PATH'] = r'C:\mingw64\bin;' + os.environ['PATH']

print("Direct DLL Test")

# Load DLL
dll_path = os.path.abspath("python/SystemMonitorNative.dll")
print(f"DLL path: {dll_path}")
print(f"DLL exists: {os.path.exists(dll_path)}")

try:
    # Try WinDLL first
    print("\nTrying WinDLL...")
    dll = ctypes.WinDLL(dll_path)
    print("WinDLL loaded successfully")
    
    # Configure function signatures
    print("\nConfiguring function signatures...")
    dll.smn_init.restype = ctypes.c_int
    dll.smn_init.argtypes = []
    
    dll.smn_get_system_info.restype = ctypes.c_char_p
    dll.smn_get_system_info.argtypes = []
    
    dll.smn_free_string.restype = None
    dll.smn_free_string.argtypes = [ctypes.c_char_p]
    
    # Test init
    print("\nTesting smn_init...")
    init_result = dll.smn_init()
    print(f"Init result: {init_result}")
    
    # Test get_system_info
    print("\nTesting smn_get_system_info...")
    result_ptr = dll.smn_get_system_info()
    print(f"Result pointer: {result_ptr}")
    
    if result_ptr:
        result_str = result_ptr.decode('utf-8', errors='ignore')
        print(f"Result string: {result_str}")
        dll.smn_free_string(result_ptr)
    else:
        print("Result is NULL!")
    
    print("Test completed successfully!")
    
except Exception as e:
    print(f"\nERROR: {e}")
    print(traceback.format_exc())
    
    # Try CDLL
    print("\nTrying CDLL instead...")
    try:
        dll = ctypes.CDLL(dll_path)
        print("CDLL loaded successfully")
        
        dll.smn_init.restype = ctypes.c_int
        result = dll.smn_init()
        print(f"Init result: {result}")
        
        dll.smn_get_system_info.restype = ctypes.c_char_p
        result_ptr = dll.smn_get_system_info()
        print(f"System info pointer: {result_ptr}")
        
        if result_ptr:
            result_str = result_ptr.decode('utf-8', errors='ignore')
            print(f"System info: {result_str}")
        
    except Exception as e2:
        print(f"CDLL also failed: {e2}")
        print(traceback.format_exc())

print("\nDone.")
input("Press Enter to exit...")