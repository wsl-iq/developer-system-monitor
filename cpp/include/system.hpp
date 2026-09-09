/**
 * @file system.hpp
 * @brief System monitoring and information retrieval.
 * @addtogroup System
 * Copyright © 2026, inc. Mohammed Al-Baqer. All rights reserved.
 */

#pragma once

#include <string>
#include <memory>
#include <mutex>
#include <chrono>
#include <vector>
#include <cstdint>

namespace smn {

struct SystemInfo {
    std::string hostname;
    std::string os_name;
    std::string os_version;
    std::string os_build;
    std::string architecture;
    uint64_t uptime_seconds;
    uint32_t processor_count;
    
    std::string to_json() const;
};

struct CPUMetrics {
    double total_usage_percent;
    std::vector<double> per_core_usage;
    uint32_t frequency_mhz;
    uint32_t base_frequency_mhz;
    double temperature_celsius;
    bool temperature_available;
    
    std::string to_json() const;
};

struct MemoryMetrics {
    uint64_t total_physical_bytes;
    uint64_t available_physical_bytes;
    uint64_t used_physical_bytes;
    double usage_percent;
    uint64_t total_page_file_bytes;
    uint64_t available_page_file_bytes;
    uint64_t total_virtual_bytes;
    uint64_t available_virtual_bytes;
    
    std::string to_json() const;
};

class SystemMonitor {
public:
    SystemMonitor();
    ~SystemMonitor();
    
    // Prevent copying
    SystemMonitor(const SystemMonitor&) = delete;
    SystemMonitor& operator=(const SystemMonitor&) = delete;
    
    // System information
    SystemInfo get_system_info();
    CPUMetrics get_cpu_metrics();
    MemoryMetrics get_memory_metrics();
    
    // Helper to convert metrics to JSON
    std::string get_all_metrics_json();
    
private:
    std::mutex mutex_;
    bool initialized_;
    
    // Performance counters
    void* cpu_counter_;
    void* memory_counter_;
    
    void initialize_counters();
    void cleanup_counters();
    
    // Helper functions
    uint64_t get_system_uptime();
    std::string get_os_version();
    std::string get_os_name();
    std::string get_os_build();
};

} // namespace smn