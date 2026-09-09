/**
 * @file exports.hpp
 * @brief C-compatible export interface for the system monitor.
 * @addtogroup SystemMonitor
 * Copyright © 2026, inc. Mohammed Al-Baqer. All rights reserved.
 */

#pragma once

// C-compatible export interface
#ifdef SYSTEMMONITORNATIVE_EXPORTS
#define SMN_API __declspec(dllexport)
#else
#define SMN_API __declspec(dllimport)
#endif

extern "C" {
    // Initialization and cleanup
    SMN_API int smn_init(void);
    SMN_API void smn_cleanup(void);
    
    // System information
    SMN_API const char* smn_get_system_info(void);
    SMN_API const char* smn_get_cpu_info(void);
    SMN_API const char* smn_get_memory_info(void);
    SMN_API const char* smn_get_disk_info(void);
    SMN_API const char* smn_get_network_info(void);
    SMN_API const char* smn_get_hardware_info(void);
    
    // Process management
    SMN_API const char* smn_get_process_list(void);
    SMN_API int smn_terminate_process(unsigned long pid);
    SMN_API int smn_suspend_process(unsigned long pid);
    SMN_API int smn_resume_process(unsigned long pid);
    
    // Services
    SMN_API const char* smn_get_service_list(void);
    SMN_API int smn_start_service(const char* service_name);
    SMN_API int smn_stop_service(const char* service_name);
    SMN_API int smn_restart_service(const char* service_name);
    
    // Real-time metrics
    SMN_API const char* smn_get_cpu_usage(void);
    SMN_API const char* smn_get_memory_usage(void);
    SMN_API const char* smn_get_disk_usage(void);
    SMN_API const char* smn_get_network_usage(void);
    
    // Cleanup functions for returned strings
    SMN_API void smn_free_string(char* str);
}