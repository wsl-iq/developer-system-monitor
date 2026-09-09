/**
 * @file cpu.hpp
 * @brief CPU monitoring and information retrieval.
 * Copyright © 2026, lnc. Mohammed Al-Baqer. All rights reserved.
 */

#pragma once

#include <string>
#include <vector>
#include <cstdint>
#include <bitset>

namespace smn {

struct CPUInfo {
    std::string manufacturer;
    std::string model;
    std::string architecture;
    uint32_t physical_cores;
    uint32_t logical_processors;
    uint32_t current_frequency_mhz;
    uint32_t max_frequency_mhz;
    std::vector<uint32_t> core_frequencies;
    std::vector<double> core_usages;
    double total_usage_percent;
    double temperature_celsius;
    bool temperature_available;
    
    std::string to_json() const;
};

class CPUMonitor {
public:
    CPUMonitor();
    ~CPUMonitor();
    
    CPUInfo get_cpu_info();
    double get_total_usage_percent();
    std::vector<double> get_per_core_usage();
    uint32_t get_current_frequency();
    uint32_t get_max_frequency();
    double get_temperature();
    
private:
    void* pdh_query_;
    void* cpu_counter_;
    void* frequency_counter_;  // Added missing member
    std::vector<void*> core_counters_;
    
    void initialize_counters();
    void cleanup_counters();
    
    std::string get_cpu_manufacturer();
    std::string get_cpu_model();
    uint32_t get_core_count();
    uint32_t get_logical_processor_count();
    
    // Helper for registry queries
    std::string query_registry_string(const std::string& key_path, const std::string& value_name);
};

} // namespace smn