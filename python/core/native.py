"""
Native interface to the C++ core library.
Uses ctypes to load and interact with the SystemMonitorNative DLL.
"""

import ctypes
import json
import logging
import os
import sys; sys.path.append(os.path.dirname(os.path.abspath(__file__)))
from pathlib import Path
from typing import Any, Dict, List, Optional

logger = logging.getLogger(__name__)
class NativeInterface:
    def __init__(self, library_path: Optional[str] = None):
        self._dll = None
        self._library_path = library_path
        self._load_library()
        
    def _load_library(self) -> None: # Load the native DLL using ctypes.     
        if self._library_path is None:
            self._library_path = self._find_library()
        
        if not self._library_path:
            logger.warning("Native library not found")
            return
        
        if not os.path.exists(self._library_path):
            logger.warning(f"Native library not found at {self._library_path}")
            return
        
        try:
            logger.info(f"Loading DLL from: {self._library_path}")
            self._dll = ctypes.WinDLL(self._library_path)
            
            # Configure function signatures
            self._configure_functions()
            
            # Initialize the library
            self._call_init()
            
            logger.info("Native library loaded successfully")
            
        except Exception as e:
            logger.error(f"Failed to load native library: {e}")
            self._dll = None
    
    def _find_library(self) -> str:
        project_root = Path(__file__).parent.parent.parent
        locations = [
            Path(__file__).parent.parent / "SystemMonitorNative.dll",
            Path(__file__).parent / "SystemMonitorNative.dll",
            project_root / "build" / "bin" / "SystemMonitorNative.dll",
            project_root / "build" / "SystemMonitorNative.dll",
            Path.cwd() / "SystemMonitorNative.dll",
        ]
        
        for location in locations:
            if location.exists():
                return str(location)
        return str(Path(__file__).parent.parent / "SystemMonitorNative.dll")
    
    def _configure_functions(self) -> None:        
        if not self._dll:
            return
        
        try:
            # System functions
            self._dll.smn_init.restype = ctypes.c_int
            self._dll.smn_init.argtypes = []
            
            self._dll.smn_cleanup.restype = None
            self._dll.smn_cleanup.argtypes = []
            
            # String-returning functions
            for func_name in [
                'smn_get_system_info',
                'smn_get_cpu_info',
                'smn_get_memory_info',
                'smn_get_disk_info',
                'smn_get_network_info',
                'smn_get_hardware_info',
                'smn_get_process_list',
                'smn_get_service_list',
                'smn_get_cpu_usage',
                'smn_get_memory_usage',
                'smn_get_disk_usage',
                'smn_get_network_usage',
            ]:
                if hasattr(self._dll, func_name):
                    func = getattr(self._dll, func_name)
                    func.restype = ctypes.c_void_p  # Use void_p instead of c_char_p
                    func.argtypes = []
            
            # Process functions
            if hasattr(self._dll, 'smn_terminate_process'):
                self._dll.smn_terminate_process.restype = ctypes.c_int
                self._dll.smn_terminate_process.argtypes = [ctypes.c_ulong]
            
            if hasattr(self._dll, 'smn_suspend_process'):
                self._dll.smn_suspend_process.restype = ctypes.c_int
                self._dll.smn_suspend_process.argtypes = [ctypes.c_ulong]
            
            if hasattr(self._dll, 'smn_resume_process'):
                self._dll.smn_resume_process.restype = ctypes.c_int
                self._dll.smn_resume_process.argtypes = [ctypes.c_ulong]
            
            # Service functions
            if hasattr(self._dll, 'smn_start_service'):
                self._dll.smn_start_service.restype = ctypes.c_int
                self._dll.smn_start_service.argtypes = [ctypes.c_char_p]
            
            if hasattr(self._dll, 'smn_stop_service'):
                self._dll.smn_stop_service.restype = ctypes.c_int
                self._dll.smn_stop_service.argtypes = [ctypes.c_char_p]
            
            if hasattr(self._dll, 'smn_restart_service'):
                self._dll.smn_restart_service.restype = ctypes.c_int
                self._dll.smn_restart_service.argtypes = [ctypes.c_char_p]
            
            # Memory management
            if hasattr(self._dll, 'smn_free_string'):
                self._dll.smn_free_string.restype = None
                self._dll.smn_free_string.argtypes = [ctypes.c_void_p]
            
        except Exception as e:
            logger.error(f"Failed to configure function signatures: {e}")
    
    def _call_init(self) -> bool:
        if not self._dll:
            return False
        
        try:
            result = self._dll.smn_init()
            return result == 0
        except Exception as e:
            logger.error(f"Failed to initialize native library: {e}")
            return False
    
    def _call_string_function(self, function_name: str) -> Optional[str]:
        if not self._dll:
            return None
        
        try:
            function = getattr(self._dll, function_name)
            
            # Call the function and get pointer
            result_ptr = function()
            
            if result_ptr and result_ptr != 0:
                # Read string from memory
                try:
                    # Try c_char_p approach first
                    result = ctypes.cast(result_ptr, ctypes.c_char_p).value
                    if result:
                        return result.decode('utf-8', errors='ignore')
                except:
                    # Try direct string_at
                    try:
                        result_bytes = ctypes.string_at(result_ptr)
                        if result_bytes:
                            return result_bytes.decode('utf-8', errors='ignore')
                    except:
                        pass
                
                # Free the memory
                try:
                    if hasattr(self._dll, 'smn_free_string'):
                        self._dll.smn_free_string(result_ptr)
                except:
                    pass
        except Exception as e:
            logger.error(f"Failed to call {function_name}: {e}")
        
        return None
    
    def _call_json_function(self, function_name: str) -> Optional[Any]:
        result = self._call_string_function(function_name)
        
        if result:
            try:
                return json.loads(result)
            except json.JSONDecodeError as e:
                logger.error(f"Failed to parse JSON from {function_name}: {e}")
                logger.debug(f"Raw data: {result[:500]}...")
        
        return None
    
    # Public API methods
    def is_available(self) -> bool:
        """Check if the native library is available."""
        return self._dll is not None
    
    def get_system_info(self) -> Optional[Dict[str, Any]]:
        return self._call_json_function('smn_get_system_info')
    
    def get_cpu_info(self) -> Optional[Dict[str, Any]]:
        return self._call_json_function('smn_get_cpu_info')
    
    def get_memory_info(self) -> Optional[Dict[str, Any]]:
        return self._call_json_function('smn_get_memory_info')
    
    def get_disk_info(self) -> Optional[List[Dict[str, Any]]]:
        return self._call_json_function('smn_get_disk_info')
    
    def get_network_info(self) -> Optional[List[Dict[str, Any]]]:
        return self._call_json_function('smn_get_network_info')
    
    def get_hardware_info(self) -> Optional[Dict[str, Any]]:
        return self._call_json_function('smn_get_hardware_info')
    
    def get_process_list(self) -> Optional[List[Dict[str, Any]]]:
        return self._call_json_function('smn_get_process_list')
    
    def get_service_list(self) -> Optional[List[Dict[str, Any]]]:
        return self._call_json_function('smn_get_service_list')
    
    def get_cpu_usage(self) -> Optional[Dict[str, Any]]:
        return self._call_json_function('smn_get_cpu_usage')
    
    def get_memory_usage(self) -> Optional[Dict[str, Any]]:
        return self._call_json_function('smn_get_memory_usage')
    
    def get_disk_usage(self) -> Optional[List[Dict[str, Any]]]:
        return self._call_json_function('smn_get_disk_usage')
    
    def get_network_usage(self) -> Optional[List[Dict[str, Any]]]:
        return self._call_json_function('smn_get_network_usage')
    
    def terminate_process(self, pid: int) -> bool:
        if not self._dll:
            return False
        try:
            result = self._dll.smn_terminate_process(ctypes.c_ulong(pid))
            return result == 0
        except Exception as e:
            logger.error(f"Failed to terminate process {pid}: {e}")
            return False
    
    def suspend_process(self, pid: int) -> bool:
        if not self._dll:
            return False
        try:
            result = self._dll.smn_suspend_process(ctypes.c_ulong(pid))
            return result == 0
        except Exception as e:
            logger.error(f"Failed to suspend process {pid}: {e}")
            return False
    
    def resume_process(self, pid: int) -> bool:
        if not self._dll:
            return False
        try:
            result = self._dll.smn_resume_process(ctypes.c_ulong(pid))
            return result == 0
        except Exception as e:
            logger.error(f"Failed to resume process {pid}: {e}")
            return False
    
    def start_service(self, service_name: str) -> bool:
        if not self._dll:
            return False
        try:
            result = self._dll.smn_start_service(service_name.encode('utf-8'))
            return result == 0
        except Exception as e:
            logger.error(f"Failed to start service {service_name}: {e}")
            return False
    
    def stop_service(self, service_name: str) -> bool:
        if not self._dll:
            return False
        try:
            result = self._dll.smn_stop_service(service_name.encode('utf-8'))
            return result == 0
        except Exception as e:
            logger.error(f"Failed to stop service {service_name}: {e}")
            return False
    
    def restart_service(self, service_name: str) -> bool:
        if not self._dll:
            return False
        try:
            result = self._dll.smn_restart_service(service_name.encode('utf-8'))
            return result == 0
        except Exception as e:
            logger.error(f"Failed to restart service {service_name}: {e}")
            return False
    
    def cleanup(self) -> None:
        if self._dll:
            try:
                self._dll.smn_cleanup()
            except Exception as e:
                logger.error(f"Failed to clean up native library: {e}")
    
    def __del__(self):
        try:
            self.cleanup()
        except:
            pass


# Singleton instance
_native_instance: Optional[NativeInterface] = None

def get_native_interface() -> NativeInterface:
    global _native_instance
    
    if _native_instance is None:
        _native_instance = NativeInterface()
    
    return _native_instance