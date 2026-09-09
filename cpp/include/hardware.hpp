/**
 * @file hardware.hpp
 * @brief Hardware monitoring and information retrieval.
 * @addtogroup Hardware
 * Copyright © 2026, inc. Mohammed Al-Baqer. All rights reserved.
 */

#pragma once

#include <string>
#include <vector>
#include <cstdint>

namespace smn {

struct HardwareInfo {
    // CPU
    struct CPUInfo {
        std::string manufacturer;
        std::string model;
        std::string architecture;
        uint32_t cores;
        uint32_t threads;
        uint32_t frequency_mhz;
        std::string socket_type;
    };
    
    // GPU
    struct GPUInfo {
        std::string name;
        std::string vendor;
        uint64_t vram_bytes;
        std::string driver_version;
        uint32_t vendor_id;
        uint32_t device_id;
    };
    
    // Motherboard
    struct MotherboardInfo {
        std::string manufacturer;
        std::string model;
        std::string version;
        std::string serial_number;
    };
    
    // BIOS
    struct BIOSInfo {
        std::string manufacturer;
        std::string version;
        std::string release_date;
        std::string smbios_version;
    };
    
    // Storage
    struct StorageDeviceInfo {
        std::string name;
        std::string model;
        std::string serial_number;
        std::string interface_type;
        std::string media_type;
        uint64_t capacity_bytes;
        bool is_ssd;
    };
    
    CPUInfo cpu;
    std::vector<GPUInfo> gpus;
    MotherboardInfo motherboard;
    BIOSInfo bios;
    std::vector<StorageDeviceInfo> storage_devices;
    
    std::string to_json() const;
};

class HardwareMonitor {
public:
    HardwareMonitor();
    ~HardwareMonitor();
    
    HardwareInfo get_hardware_info();
    
private:
    // Query methods
    HardwareInfo::CPUInfo query_cpu_info();
    std::vector<HardwareInfo::GPUInfo> query_gpu_info();
    HardwareInfo::MotherboardInfo query_motherboard_info();
    HardwareInfo::BIOSInfo query_bios_info();
    std::vector<HardwareInfo::StorageDeviceInfo> query_storage_devices();
    
    // Registry helper
    std::string query_registry_string(const std::string& key_path, 
                                      const std::string& value_name);
    
    // WMI helper for some queries
    std::string query_wmi(const std::string& wql_query, 
                          const std::string& property);
    
    // DXGI for GPU information
    void query_dxgi_gpus(std::vector<HardwareInfo::GPUInfo>& gpus);
    
    // SMBIOS for system information
    void query_smbios_data();
};

} // namespace smn