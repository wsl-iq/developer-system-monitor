/**
 * @file memory.hpp
 * @brief Memory monitoring and information retrieval.
 * @addtogroup Memory
 * Copyright © 2026, inc. Mohammed Al-Baqer. All rights reserved.
 */

#pragma once

#include <string>
#include <cstdint>
#include <vector>

namespace smn {

struct MemoryInfo {
    uint64_t total_physical_bytes;
    uint64_t available_physical_bytes;
    uint64_t used_physical_bytes;
    double usage_percent;
    uint64_t total_page_file_bytes;
    uint64_t available_page_file_bytes;
    uint64_t total_virtual_bytes;
    uint64_t available_virtual_bytes;
    
    // Hardware information
    struct MemoryModule {
        std::string manufacturer;
        std::string part_number;
        uint64_t capacity_bytes;
        uint32_t speed_mhz;
        std::string type;
    };
    
    std::vector<MemoryModule> modules;
    
    std::string to_json() const;
};

class MemoryMonitor {
public:
    MemoryMonitor();
    ~MemoryMonitor();
    
    MemoryInfo get_memory_info();
    double get_usage_percent();
    
private:
    std::vector<MemoryInfo::MemoryModule> get_memory_modules();
    void* pdh_query_;
    void* memory_counter_;
    
    void initialize_counters();
    void cleanup_counters();
    
    // SMBIOS memory information
    void query_smbios_memory();
};

} // namespace smn