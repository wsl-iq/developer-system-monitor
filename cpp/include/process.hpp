/**
 * @file process.hpp
 * @brief Process monitoring and information retrieval.
 * @addtogroup Process
 * Copyright © 2026, inc. Mohammed Al-Baqer. All rights reserved.
 */

#pragma once

#include <string>
#include <vector>
#include <cstdint>
#include <chrono>
#include <unordered_map>

namespace smn {

struct ProcessInfo {
    uint32_t pid;
    uint32_t parent_pid;
    std::string name;
    std::string executable_path;
    std::string user_name;
    std::string status;
    uint32_t priority_class;
    uint64_t memory_usage_bytes;
    uint64_t peak_memory_usage_bytes;
    double cpu_usage_percent;
    uint64_t create_time;
    uint32_t thread_count;
    uint64_t disk_read_bytes;
    uint64_t disk_write_bytes;
    
    std::string to_json() const;
};

class ProcessManager {
public:
    ProcessManager();
    ~ProcessManager();
    
    std::vector<ProcessInfo> get_process_list();
    ProcessInfo get_process_info(uint32_t pid);
    
    bool terminate_process(uint32_t pid);
    bool suspend_process(uint32_t pid);
    bool resume_process(uint32_t pid);
    bool set_process_priority(uint32_t pid, uint32_t priority_class);
    
    double get_process_cpu_usage(uint32_t pid);
    uint64_t get_process_memory(uint32_t pid);
    std::string get_process_path(uint32_t pid);
    
private:
    void* pdh_query_;
    std::chrono::steady_clock::time_point last_sample_time_;
    
    void initialize_counters();
    void cleanup_counters();
    
    // Helper functions
    std::string get_process_user(uint32_t pid);
    uint32_t get_process_parent(uint32_t pid);
    std::vector<uint32_t> enumerate_process_ids();
    
    struct ProcessCPUSample {
        uint64_t cpu_time;
        std::chrono::steady_clock::time_point sample_time;
    };
    
    std::unordered_map<uint32_t, ProcessCPUSample> process_cpu_samples_;
};

} // namespace smn