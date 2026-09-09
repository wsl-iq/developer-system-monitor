/**
 * @file disk.hpp
 * @brief Disk monitoring and information retrieval.
 * @addtogroup Disk
 * Copyright © 2026, inc. Mohammed Al-Baqer. All rights reserved.
 */

#pragma once

#include <string>
#include <vector>
#include <cstdint>

namespace smn {

struct DiskInfo {
    std::string drive_letter;      // e.g., "C:"
    std::string volume_name;      // e.g., "System"
    std::string file_system;     // e.g., "NTFS"
    std::string drive_type;     // e.g., "Local Disk"
    uint64_t total_bytes;      // Total size in bytes
    uint64_t free_bytes;      // Free space in bytes
    uint64_t used_bytes;     // Used space in bytes
    double usage_percent;   // Usage percentage
    
    // Performance metrics
    uint64_t read_bytes_per_sec;    // Read speed in bytes per second
    uint64_t write_bytes_per_sec;  // Write speed in bytes per second
    uint64_t total_read_bytes;    // Total read bytes since system start
    uint64_t total_write_bytes;  // Total write bytes since system start
    
    std::string to_json() const; // Convert DiskInfo to JSON representation
};

class DiskMonitor {
public:
    DiskMonitor();
    ~DiskMonitor();
    
    std::vector<DiskInfo> get_disk_list();
    std::vector<DiskInfo> get_disk_usage();
    
private:
    void* pdh_query_;
    std::vector<void*> disk_counters_;
    
    void initialize_counters();
    void cleanup_counters();
    
    std::vector<std::string> enumerate_drives();
    uint64_t get_disk_read_bytes(const std::string& drive);
    uint64_t get_disk_write_bytes(const std::string& drive);
};

} // namespace smn