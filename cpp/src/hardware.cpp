/**
 * @file hardware.hpp
 * @brief Hardware monitoring and information retrieval.
 * @addtogroup Hardware
 * Copyright © 2026, inc. Mohammed Al-Baqer. All rights reserved.
 */

#include "hardware.hpp"
#include <windows.h>
#include <d3d11.h>
#include <dxgi.h>
#include <sstream>
#include <iomanip>
#include <comdef.h>
#include <winioctl.h>

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "ole32.lib")
#pragma comment(lib, "oleaut32.lib")

namespace smn {

HardwareMonitor::HardwareMonitor() {
}

HardwareMonitor::~HardwareMonitor() {
}

HardwareInfo HardwareMonitor::get_hardware_info() {
    HardwareInfo info;
    
    info.cpu = query_cpu_info();
    info.gpus = query_gpu_info();
    info.motherboard = query_motherboard_info();
    info.bios = query_bios_info();
    info.storage_devices = query_storage_devices();
    
    return info;
}

HardwareInfo::CPUInfo HardwareMonitor::query_cpu_info() {
    HardwareInfo::CPUInfo info;
    
    HKEY hKey;
    if (RegOpenKeyExA(HKEY_LOCAL_MACHINE, 
                      "HARDWARE\\DESCRIPTION\\System\\CentralProcessor\\0",
                      0, KEY_READ, &hKey) == ERROR_SUCCESS) {
        char value[256];
        DWORD size = sizeof(value);
        
        if (RegQueryValueExA(hKey, "VendorIdentifier", nullptr, nullptr,
                            (LPBYTE)value, &size) == ERROR_SUCCESS) {
            info.manufacturer = value;
        }
        
        size = sizeof(value);
        if (RegQueryValueExA(hKey, "ProcessorNameString", nullptr, nullptr,
                            (LPBYTE)value, &size) == ERROR_SUCCESS) {
            info.model = value;
        }
        
        DWORD mhz;
        size = sizeof(mhz);
        if (RegQueryValueExA(hKey, "~MHz", nullptr, nullptr,
                            (LPBYTE)&mhz, &size) == ERROR_SUCCESS) {
            info.frequency_mhz = mhz;
        }
        
        RegCloseKey(hKey);
    }
    
    SYSTEM_INFO sys_info;
    GetNativeSystemInfo(&sys_info);
    info.cores = sys_info.dwNumberOfProcessors;
    info.threads = sys_info.dwNumberOfProcessors;
    
    switch (sys_info.wProcessorArchitecture) {
        case PROCESSOR_ARCHITECTURE_AMD64:
            info.architecture = "x64";
            break;
        case PROCESSOR_ARCHITECTURE_INTEL:
            info.architecture = "x86";
            break;
        case PROCESSOR_ARCHITECTURE_ARM64:
            info.architecture = "ARM64";
            break;
        default:
            info.architecture = "Unknown";
    }
    
    return info;
}

std::vector<HardwareInfo::GPUInfo> HardwareMonitor::query_gpu_info() {
    std::vector<HardwareInfo::GPUInfo> gpus;
    
    query_dxgi_gpus(gpus);
    
    return gpus;
}

void HardwareMonitor::query_dxgi_gpus(std::vector<HardwareInfo::GPUInfo>& gpus) {
    IDXGIFactory* factory = nullptr;
    HRESULT hr = CreateDXGIFactory(__uuidof(IDXGIFactory), (void**)&factory);
    
    if (SUCCEEDED(hr)) {
        UINT adapter_index = 0;
        IDXGIAdapter* adapter = nullptr;
        
        while (factory->EnumAdapters(adapter_index, &adapter) != DXGI_ERROR_NOT_FOUND) {
            DXGI_ADAPTER_DESC desc;
            if (SUCCEEDED(adapter->GetDesc(&desc))) {
                HardwareInfo::GPUInfo info;
                
                // Convert wide string to narrow
                char name[256];
                WideCharToMultiByte(CP_UTF8, 0, desc.Description, -1,
                                   name, 256, nullptr, nullptr);
                info.name = name;
                
                info.vendor_id = desc.VendorId;
                info.device_id = desc.DeviceId;
                info.vram_bytes = desc.DedicatedVideoMemory;
                
                // Determine vendor
                switch (desc.VendorId) {
                    case 0x10DE: info.vendor = "NVIDIA"; break;
                    case 0x1002: info.vendor = "AMD"; break;
                    case 0x8086: info.vendor = "Intel"; break;
                    case 0x1414: info.vendor = "Microsoft"; break;
                    default: info.vendor = "Unknown";
                }
                
                gpus.push_back(info);
            }
            
            adapter->Release();
            adapter_index++;
        }
        
        factory->Release();
    }
}

HardwareInfo::MotherboardInfo HardwareMonitor::query_motherboard_info() {
    HardwareInfo::MotherboardInfo info;
    
    HKEY hKey;
    if (RegOpenKeyExA(HKEY_LOCAL_MACHINE, 
                      "HARDWARE\\DESCRIPTION\\System\\BIOS",
                      0, KEY_READ, &hKey) == ERROR_SUCCESS) {
        char value[256];
        DWORD size = sizeof(value);
        
        if (RegQueryValueExA(hKey, "BaseBoardManufacturer", nullptr, nullptr,
                            (LPBYTE)value, &size) == ERROR_SUCCESS) {
            info.manufacturer = value;
        }
        
        size = sizeof(value);
        if (RegQueryValueExA(hKey, "BaseBoardProduct", nullptr, nullptr,
                            (LPBYTE)value, &size) == ERROR_SUCCESS) {
            info.model = value;
        }
        
        size = sizeof(value);
        if (RegQueryValueExA(hKey, "BaseBoardVersion", nullptr, nullptr,
                            (LPBYTE)value, &size) == ERROR_SUCCESS) {
            info.version = value;
        }
        
        RegCloseKey(hKey);
    }
    
    return info;
}

HardwareInfo::BIOSInfo HardwareMonitor::query_bios_info() {
    HardwareInfo::BIOSInfo info;
    
    HKEY hKey;
    if (RegOpenKeyExA(HKEY_LOCAL_MACHINE, 
                      "HARDWARE\\DESCRIPTION\\System\\BIOS",
                      0, KEY_READ, &hKey) == ERROR_SUCCESS) {
        char value[256];
        DWORD size = sizeof(value);
        
        if (RegQueryValueExA(hKey, "BIOSVendor", nullptr, nullptr,
                            (LPBYTE)value, &size) == ERROR_SUCCESS) {
            info.manufacturer = value;
        }
        
        size = sizeof(value);
        if (RegQueryValueExA(hKey, "BIOSVersion", nullptr, nullptr,
                            (LPBYTE)value, &size) == ERROR_SUCCESS) {
            info.version = value;
        }
        
        size = sizeof(value);
        if (RegQueryValueExA(hKey, "BIOSReleaseDate", nullptr, nullptr,
                            (LPBYTE)value, &size) == ERROR_SUCCESS) {
            info.release_date = value;
        }
        
        RegCloseKey(hKey);
    }
    
    return info;
}

std::vector<HardwareInfo::StorageDeviceInfo> HardwareMonitor::query_storage_devices() {
    std::vector<HardwareInfo::StorageDeviceInfo> devices;
    
    // Use GetLogicalDrives and GetDiskFreeSpaceEx for storage info
    DWORD drives = GetLogicalDrives();
    if (drives == 0) {
        return devices;
    }
    
    for (int i = 0; i < 26; ++i) {
        if (drives & (1 << i)) {
            std::string drive_letter(1, 'A' + i);
            drive_letter += ":\\";
            
            UINT drive_type = GetDriveTypeA(drive_letter.c_str());
            if (drive_type == DRIVE_FIXED || drive_type == DRIVE_REMOVABLE) {
                HardwareInfo::StorageDeviceInfo info;
                
                info.name = "Drive " + drive_letter.substr(0, 2);
                info.model = drive_letter.substr(0, 2);
                info.interface_type = (drive_type == DRIVE_FIXED) ? "SATA" : "USB";
                info.media_type = (drive_type == DRIVE_FIXED) ? "HDD/SSD" : "Removable";
                info.is_ssd = false;  // Would need additional queries to determine
                
                // Get capacity
                ULARGE_INTEGER free_bytes_available, total_bytes, total_free_bytes;
                if (GetDiskFreeSpaceExA(drive_letter.c_str(), 
                                        &free_bytes_available,
                                        &total_bytes, &total_free_bytes)) {
                    info.capacity_bytes = total_bytes.QuadPart;
                }
                
                if (info.capacity_bytes > 0) {
                    devices.push_back(info);
                }
            }
        }
    }
    
    return devices;
}

std::string HardwareMonitor::query_registry_string(const std::string& key_path, 
                                                    const std::string& value_name) {
    std::string result;
    HKEY hKey;
    
    if (RegOpenKeyExA(HKEY_LOCAL_MACHINE, key_path.c_str(), 0, 
                      KEY_READ, &hKey) == ERROR_SUCCESS) {
        char value[256];
        DWORD size = sizeof(value);
        DWORD type;
        
        if (RegQueryValueExA(hKey, value_name.c_str(), nullptr, &type,
                            (LPBYTE)value, &size) == ERROR_SUCCESS) {
            if (type == REG_SZ || type == REG_EXPAND_SZ) {
                result = value;
            }
        }
        RegCloseKey(hKey);
    }
    
    return result;
}

std::string HardwareMonitor::query_wmi(const std::string& wql_query, 
                                        const std::string& property) {
    // WMI query implementation will be added in future phases
    return "";
}

std::string HardwareInfo::to_json() const {
    std::ostringstream oss;
    oss << "{"
        << "\"cpu\":{"
        << "\"manufacturer\":\"" << cpu.manufacturer << "\","
        << "\"model\":\"" << cpu.model << "\","
        << "\"architecture\":\"" << cpu.architecture << "\","
        << "\"cores\":" << cpu.cores << ","
        << "\"threads\":" << cpu.threads << ","
        << "\"frequency_mhz\":" << cpu.frequency_mhz
        << "},";
    
    oss << "\"gpus\":[";
    for (size_t i = 0; i < gpus.size(); ++i) {
        if (i > 0) oss << ",";
        oss << "{"
            << "\"name\":\"" << gpus[i].name << "\","
            << "\"vendor\":\"" << gpus[i].vendor << "\","
            << "\"vram_bytes\":" << gpus[i].vram_bytes << ","
            << "\"vendor_id\":" << gpus[i].vendor_id << ","
            << "\"device_id\":" << gpus[i].device_id
            << "}";
    }
    oss << "],";
    
    oss << "\"motherboard\":{"
        << "\"manufacturer\":\"" << motherboard.manufacturer << "\","
        << "\"model\":\"" << motherboard.model << "\","
        << "\"version\":\"" << motherboard.version << "\""
        << "},";
    
    oss << "\"bios\":{"
        << "\"manufacturer\":\"" << bios.manufacturer << "\","
        << "\"version\":\"" << bios.version << "\","
        << "\"release_date\":\"" << bios.release_date << "\""
        << "},";
    
    oss << "\"storage_devices\":[";
    for (size_t i = 0; i < storage_devices.size(); ++i) {
        if (i > 0) oss << ",";
        oss << "{"
            << "\"name\":\"" << storage_devices[i].name << "\","
            << "\"model\":\"" << storage_devices[i].model << "\","
            << "\"capacity_bytes\":" << storage_devices[i].capacity_bytes << ","
            << "\"interface_type\":\"" << storage_devices[i].interface_type << "\","
            << "\"media_type\":\"" << storage_devices[i].media_type << "\","
            << "\"is_ssd\":" << (storage_devices[i].is_ssd ? "true" : "false")
            << "}";
    }
    oss << "]";
    
    oss << "}";
    return oss.str();
}

} // namespace smn