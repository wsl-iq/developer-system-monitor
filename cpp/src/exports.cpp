/**
 * @file exports.cpp
 * @brief exports monitoring and information retrieval.
 * @addtogroup exports
 * Copyright © 2026, inc. Mohammed Al-Baqer. All rights reserved.
 */

#include "exports.hpp"
#include "cpu.hpp"
#include "memory.hpp"
#include "process.hpp"
#include "disk.hpp"
#include "network.hpp"
#include "services.hpp"
#include "hardware.hpp"
#include "system.hpp"

#include <memory>
#include <string>
#include <cstring>

// Global instances (singletons)
static std::unique_ptr<smn::SystemMonitor> g_system_monitor;
static std::unique_ptr<smn::CPUMonitor> g_cpu_monitor;
static std::unique_ptr<smn::MemoryMonitor> g_memory_monitor;
static std::unique_ptr<smn::ProcessManager> g_process_manager;
static std::unique_ptr<smn::DiskMonitor> g_disk_monitor;
static std::unique_ptr<smn::NetworkMonitor> g_network_monitor;
static std::unique_ptr<smn::ServiceManager> g_service_manager;
static std::unique_ptr<smn::HardwareMonitor> g_hardware_monitor;

static bool g_initialized = false;

// Helper to allocate and return string - use new[] instead of strcpy_s
static char* allocate_string(const std::string& str) {
    const size_t len = str.length();
    char* result = new char[len + 1];
    memcpy(result, str.c_str(), len);
    result[len] = '\0';
    return result;
}

// Initialization and cleanup
SMN_API int smn_init(void) {
    try {
        if (g_initialized) return 0;
        
        // Initialize only system monitor first (most basic)
        g_system_monitor = std::make_unique<smn::SystemMonitor>();
        
        // Initialize others lazily - they will be created on first use
        g_initialized = true;
        return 0;
    }
    catch (...) {
        return -1;
    }
}

SMN_API void smn_cleanup(void) {
    g_system_monitor.reset();
    g_cpu_monitor.reset();
    g_memory_monitor.reset();
    g_process_manager.reset();
    g_disk_monitor.reset();
    g_network_monitor.reset();
    g_service_manager.reset();
    g_hardware_monitor.reset();
    g_initialized = false;
}

// Helper to ensure CPU monitor exists
static bool ensure_cpu_monitor() {
    if (!g_cpu_monitor) {
        try {
            g_cpu_monitor = std::make_unique<smn::CPUMonitor>();
        } catch (...) {
            return false;
        }
    }
    return g_cpu_monitor != nullptr;
}

// Helper to ensure memory monitor exists
static bool ensure_memory_monitor() {
    if (!g_memory_monitor) {
        try {
            g_memory_monitor = std::make_unique<smn::MemoryMonitor>();
        } catch (...) {
            return false;
        }
    }
    return g_memory_monitor != nullptr;
}

// Helper to ensure process manager exists
static bool ensure_process_manager() {
    if (!g_process_manager) {
        try {
            g_process_manager = std::make_unique<smn::ProcessManager>();
        } catch (...) {
            return false;
        }
    }
    return g_process_manager != nullptr;
}

// Helper to ensure disk monitor exists
static bool ensure_disk_monitor() {
    if (!g_disk_monitor) {
        try {
            g_disk_monitor = std::make_unique<smn::DiskMonitor>();
        } catch (...) {
            return false;
        }
    }
    return g_disk_monitor != nullptr;
}

// Helper to ensure network monitor exists
static bool ensure_network_monitor() {
    if (!g_network_monitor) {
        try {
            g_network_monitor = std::make_unique<smn::NetworkMonitor>();
        } catch (...) {
            return false;
        }
    }
    return g_network_monitor != nullptr;
}

// Helper to ensure service manager exists
static bool ensure_service_manager() {
    if (!g_service_manager) {
        try {
            g_service_manager = std::make_unique<smn::ServiceManager>();
        } catch (...) {
            return false;
        }
    }
    return g_service_manager != nullptr;
}

// Helper to ensure hardware monitor exists
static bool ensure_hardware_monitor() {
    if (!g_hardware_monitor) {
        try {
            g_hardware_monitor = std::make_unique<smn::HardwareMonitor>();
        } catch (...) {
            return false;
        }
    }
    return g_hardware_monitor != nullptr;
}

// System information
SMN_API const char* smn_get_system_info(void) {
    if (!g_system_monitor) {
        try {
            g_system_monitor = std::make_unique<smn::SystemMonitor>();
        } catch (...) {
            return nullptr;
        }
    }
    
    try {
        auto info = g_system_monitor->get_system_info();
        return allocate_string(info.to_json());
    } catch (...) {
        return allocate_string("{}");
    }
}

SMN_API const char* smn_get_cpu_info(void) {
    if (!ensure_cpu_monitor()) return allocate_string("{}");
    
    try {
        auto info = g_cpu_monitor->get_cpu_info();
        return allocate_string(info.to_json());
    } catch (...) {
        return allocate_string("{}");
    }
}

SMN_API const char* smn_get_memory_info(void) {
    if (!ensure_memory_monitor()) return allocate_string("{}");
    
    try {
        auto info = g_memory_monitor->get_memory_info();
        return allocate_string(info.to_json());
    } catch (...) {
        return allocate_string("{}");
    }
}

SMN_API const char* smn_get_disk_info(void) {
    if (!ensure_disk_monitor()) return allocate_string("[]");
    
    try {
        auto disks = g_disk_monitor->get_disk_list();
        
        std::string json = "[";
        for (size_t i = 0; i < disks.size(); ++i) {
            if (i > 0) json += ",";
            json += disks[i].to_json();
        }
        json += "]";
        
        return allocate_string(json);
    } catch (...) {
        return allocate_string("[]");
    }
}

SMN_API const char* smn_get_network_info(void) {
    if (!ensure_network_monitor()) return allocate_string("[]");
    
    try {
        auto adapters = g_network_monitor->get_network_adapters();
        
        std::string json = "[";
        for (size_t i = 0; i < adapters.size(); ++i) {
            if (i > 0) json += ",";
            json += adapters[i].to_json();
        }
        json += "]";
        
        return allocate_string(json);
    } catch (...) {
        return allocate_string("[]");
    }
}

SMN_API const char* smn_get_hardware_info(void) {
    if (!ensure_hardware_monitor()) return allocate_string("{}");
    
    try {
        auto info = g_hardware_monitor->get_hardware_info();
        return allocate_string(info.to_json());
    } catch (...) {
        return allocate_string("{}");
    }
}

// Process management
SMN_API const char* smn_get_process_list(void) {
    if (!ensure_process_manager()) return allocate_string("[]");
    
    try {
        auto processes = g_process_manager->get_process_list();
        
        std::string json = "[";
        for (size_t i = 0; i < processes.size(); ++i) {
            if (i > 0) json += ",";
            json += processes[i].to_json();
        }
        json += "]";
        
        return allocate_string(json);
    } catch (...) {
        return allocate_string("[]");
    }
}

SMN_API int smn_terminate_process(unsigned long pid) {
    if (!ensure_process_manager()) return -1;
    
    try {
        return g_process_manager->terminate_process(pid) ? 0 : -1;
    } catch (...) {
        return -1;
    }
}

SMN_API int smn_suspend_process(unsigned long pid) {
    if (!ensure_process_manager()) return -1;
    
    try {
        return g_process_manager->suspend_process(pid) ? 0 : -1;
    } catch (...) {
        return -1;
    }
}

SMN_API int smn_resume_process(unsigned long pid) {
    if (!ensure_process_manager()) return -1;
    
    try {
        return g_process_manager->resume_process(pid) ? 0 : -1;
    } catch (...) {
        return -1;
    }
}

// Services
SMN_API const char* smn_get_service_list(void) {
    if (!ensure_service_manager()) return allocate_string("[]");
    
    try {
        auto services = g_service_manager->get_service_list();
        
        std::string json = "[";
        for (size_t i = 0; i < services.size(); ++i) {
            if (i > 0) json += ",";
            json += services[i].to_json();
        }
        json += "]";
        
        return allocate_string(json);
    } catch (...) {
        return allocate_string("[]");
    }
}

SMN_API int smn_start_service(const char* service_name) {
    if (!ensure_service_manager() || !service_name) return -1;
    
    try {
        return g_service_manager->start_service(service_name) ? 0 : -1;
    } catch (...) {
        return -1;
    }
}

SMN_API int smn_stop_service(const char* service_name) {
    if (!ensure_service_manager() || !service_name) return -1;
    
    try {
        return g_service_manager->stop_service(service_name) ? 0 : -1;
    } catch (...) {
        return -1;
    }
}

SMN_API int smn_restart_service(const char* service_name) {
    if (!ensure_service_manager() || !service_name) return -1;
    
    try {
        return g_service_manager->restart_service(service_name) ? 0 : -1;
    } catch (...) {
        return -1;
    }
}

// Real-time metrics
SMN_API const char* smn_get_cpu_usage(void) {
    if (!ensure_cpu_monitor()) return allocate_string("{\"usage_percent\":0}");
    
    try {
        auto usage = g_cpu_monitor->get_total_usage_percent();
        std::string json = "{\"usage_percent\":" + std::to_string(usage) + "}";
        return allocate_string(json);
    } catch (...) {
        return allocate_string("{\"usage_percent\":0}");
    }
}

SMN_API const char* smn_get_memory_usage(void) {
    if (!ensure_memory_monitor()) return allocate_string("{}");
    
    try {
        auto memory = g_memory_monitor->get_memory_info();
        return allocate_string(memory.to_json());
    } catch (...) {
        return allocate_string("{}");
    }
}

SMN_API const char* smn_get_disk_usage(void) {
    if (!ensure_disk_monitor()) return allocate_string("[]");
    
    try {
        auto disks = g_disk_monitor->get_disk_usage();
        
        std::string json = "[";
        for (size_t i = 0; i < disks.size(); ++i) {
            if (i > 0) json += ",";
            json += disks[i].to_json();
        }
        json += "]";
        
        return allocate_string(json);
    } catch (...) {
        return allocate_string("[]");
    }
}

SMN_API const char* smn_get_network_usage(void) {
    if (!ensure_network_monitor()) return allocate_string("[]");
    
    try {
        auto adapters = g_network_monitor->get_network_usage();
        
        std::string json = "[";
        for (size_t i = 0; i < adapters.size(); ++i) {
            if (i > 0) json += ",";
            json += adapters[i].to_json();
        }
        json += "]";
        
        return allocate_string(json);
    } catch (...) {
        return allocate_string("[]");
    }
}

// Cleanup functions for returned strings
SMN_API void smn_free_string(char* str) {
    delete[] str;
}