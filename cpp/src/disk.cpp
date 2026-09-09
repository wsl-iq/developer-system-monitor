/**
 * @file disk.cpp
 * @brief Disk monitoring and information retrieval.
 * @addtogroup Disk
 * Copyright © 2026, inc. Mohammed Al-Baqer. All rights reserved.
 */

#include "disk.hpp"
#include <windows.h>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <cstring>
#include <cstdio>

namespace smn {

// JSON escape helper
static std::string json_escape(const std::string& str) {
    std::string result;
    result.reserve(str.length() * 2);

    for (char c : str) {
        switch (c) {
            case '"': result += "\\\""; break;
            case '\\': result += "\\\\"; break;
            case '\n': result += "\\n"; break;
            case '\r': result += "\\r"; break;
            case '\t': result += "\\t"; break;
            default:
                if (static_cast<unsigned char>(c) < 0x20) {
                    char buf[8];
                    std::snprintf(buf, sizeof(buf), "\\u%04x", static_cast<unsigned char>(c));
                    result += buf;
                } else {
                    result += c;
                }
        }
    }

    return result;
}

DiskMonitor::DiskMonitor() : pdh_query_(nullptr) {
    initialize_counters();
}

DiskMonitor::~DiskMonitor() {
    cleanup_counters();
}

void DiskMonitor::initialize_counters() {
    // Initialize disk performance counters
}

void DiskMonitor::cleanup_counters() {
    if (pdh_query_) {
        // Cleanup PDH query
        pdh_query_ = nullptr;
    }
}

std::vector<DiskInfo> DiskMonitor::get_disk_list() {
    std::vector<DiskInfo> disks;
    
    DWORD drives = GetLogicalDrives();
    if (drives == 0) {
        return disks;
    }
    
    for (int i = 0; i < 26; ++i) {
        if (drives & (1 << i)) {
            std::string drive_letter(1, 'A' + i);
            drive_letter += ":\\";
            
            UINT drive_type = GetDriveTypeA(drive_letter.c_str());
            if (drive_type == DRIVE_FIXED || drive_type == DRIVE_REMOVABLE) {
                DiskInfo info;
                info.drive_letter = drive_letter.substr(0, 2);
                
                // Get volume information
                char volume_name[MAX_PATH];
                char file_system[MAX_PATH];
                DWORD serial_number, max_component_len, file_system_flags;
                
                if (GetVolumeInformationA(drive_letter.c_str(), volume_name,
                                         MAX_PATH, &serial_number, 
                                         &max_component_len, &file_system_flags,
                                         file_system, MAX_PATH)) {
                    info.volume_name = volume_name;
                    info.file_system = file_system;
                }
                
                switch (drive_type) {
                    case DRIVE_FIXED:
                        info.drive_type = "Fixed";
                        break;
                    case DRIVE_REMOVABLE:
                        info.drive_type = "Removable";
                        break;
                    case DRIVE_REMOTE:
                        info.drive_type = "Network";
                        break;
                    case DRIVE_CDROM:
                        info.drive_type = "CD-ROM";
                        break;
                    default:
                        info.drive_type = "Unknown";
                }
                
                // Get capacity
                ULARGE_INTEGER free_bytes_available, total_bytes, total_free_bytes;
                if (GetDiskFreeSpaceExA(drive_letter.c_str(), 
                                        &free_bytes_available,
                                        &total_bytes, &total_free_bytes)) {
                    info.total_bytes = total_bytes.QuadPart;
                    info.free_bytes = total_free_bytes.QuadPart;
                    info.used_bytes = total_bytes.QuadPart - total_free_bytes.QuadPart;
                    info.usage_percent = (double)info.used_bytes / 
                                        info.total_bytes * 100.0;
                }
                
                disks.push_back(info);
            }
        }
    }
    
    return disks;
}

std::vector<DiskInfo> DiskMonitor::get_disk_usage() {
    auto disks = get_disk_list();
    
    // Add real-time read/write statistics
    // This will be implemented with PDH in the full version
    for (auto& disk : disks) {
        disk.read_bytes_per_sec = 0;
        disk.write_bytes_per_sec = 0;
        disk.total_read_bytes = 0;
        disk.total_write_bytes = 0;
    }
    
    return disks;
}

std::vector<std::string> DiskMonitor::enumerate_drives() {
    std::vector<std::string> drives;
    DWORD drives_mask = GetLogicalDrives();
    
    for (int i = 0; i < 26; ++i) {
        if (drives_mask & (1 << i)) {
            std::string drive(1, 'A' + i);
            drive += ":";
            drives.push_back(drive);
        }
    }
    
    return drives;
}

uint64_t DiskMonitor::get_disk_read_bytes(const std::string& drive) {
    // Placeholder - will be implemented with performance counters
    return 0;
}

uint64_t DiskMonitor::get_disk_write_bytes(const std::string& drive) {
    // Placeholder - will be implemented with performance counters
    return 0;
}

std::string DiskInfo::to_json() const {
    std::ostringstream oss;
    oss << "{"
        << "\"drive_letter\":\"" << drive_letter << "\","
        << "\"volume_name\":\"" << volume_name << "\","
        << "\"file_system\":\"" << file_system << "\","
        << "\"drive_type\":\"" << drive_type << "\","
        << "\"total_bytes\":" << total_bytes << ","
        << "\"free_bytes\":" << free_bytes << ","
        << "\"used_bytes\":" << used_bytes << ","
        << "\"usage_percent\":" << std::fixed << std::setprecision(2) 
        << usage_percent << ","
        << "\"read_bytes_per_sec\":" << read_bytes_per_sec << ","
        << "\"write_bytes_per_sec\":" << write_bytes_per_sec << ","
        << "\"total_read_bytes\":" << total_read_bytes << ","
        << "\"total_write_bytes\":" << total_write_bytes
        << "}";
    return oss.str();
}

} // namespace smn