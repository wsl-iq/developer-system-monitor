/**
 * @file memory.cpp
 * @brief Memory monitoring and information retrieval.
 * @addtogroup Memory
 * Copyright © 2026, inc. Mohammed Al-Baqer. All rights reserved.
 */

#include "memory.hpp"
#include <windows.h>
#include <psapi.h>
#include <sstream>
#include <iomanip>
#include <vector>
#include <comdef.h>
#include <wbemidl.h>

#pragma comment(lib, "psapi.lib")
#pragma comment(lib, "wbemuuid.lib")
#pragma comment(lib, "ole32.lib")
#pragma comment(lib, "oleaut32.lib")

namespace smn {

// Helper function to convert BSTR to std::string
static std::string BSTRToString(BSTR bstr) {
    if (!bstr) return "";
    
    int len = SysStringLen(bstr);
    if (len == 0) return "";
    
    // Convert UTF-16 to UTF-8
    int utf8_len = WideCharToMultiByte(CP_UTF8, 0, bstr, len, nullptr, 0, nullptr, nullptr);
    if (utf8_len == 0) return "";
    
    std::string result(utf8_len, 0);
    WideCharToMultiByte(CP_UTF8, 0, bstr, len, &result[0], utf8_len, nullptr, nullptr);
    
    return result;
}

MemoryMonitor::MemoryMonitor() {
    initialize_counters();
}

MemoryMonitor::~MemoryMonitor() {
    cleanup_counters();
}

void MemoryMonitor::initialize_counters() {
    // Performance counters can be added here if needed
    // For now, we use GlobalMemoryStatusEx which is simpler and reliable
}

void MemoryMonitor::cleanup_counters() {
    // Cleanup if needed
}

MemoryInfo MemoryMonitor::get_memory_info() {
    MemoryInfo info;
    
    // Use GlobalMemoryStatusEx for basic memory info
    MEMORYSTATUSEX mem_status;
    mem_status.dwLength = sizeof(mem_status);
    
    if (GlobalMemoryStatusEx(&mem_status)) {
        info.total_physical_bytes = mem_status.ullTotalPhys;
        info.available_physical_bytes = mem_status.ullAvailPhys;
        info.used_physical_bytes = mem_status.ullTotalPhys - mem_status.ullAvailPhys;
        
        if (info.total_physical_bytes > 0) {
            info.usage_percent = static_cast<double>(info.used_physical_bytes) / 
                                info.total_physical_bytes * 100.0;
        }
        
        info.total_page_file_bytes = mem_status.ullTotalPageFile;
        info.available_page_file_bytes = mem_status.ullAvailPageFile;
        info.total_virtual_bytes = mem_status.ullTotalVirtual;
        info.available_virtual_bytes = mem_status.ullAvailVirtual;
    }
    
    // Get performance information for more details
    PERFORMANCE_INFORMATION perf_info;
    perf_info.cb = sizeof(perf_info);
    
    if (GetPerformanceInfo(&perf_info, sizeof(perf_info))) {
        // Additional memory metrics can be calculated here
    }
    
    // Get memory modules
    info.modules = get_memory_modules();
    
    return info;
}

double MemoryMonitor::get_usage_percent() {
    auto info = get_memory_info();
    return info.usage_percent;
}

std::vector<MemoryInfo::MemoryModule> MemoryMonitor::get_memory_modules() {
    std::vector<MemoryInfo::MemoryModule> modules;
    
    // Query WMI for physical memory modules
    HRESULT hr = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
    if (FAILED(hr)) return modules;
    
    IWbemLocator* locator = nullptr;
    hr = CoCreateInstance(CLSID_WbemLocator, nullptr, CLSCTX_INPROC_SERVER,
                         IID_IWbemLocator, (void**)&locator);
    
    if (SUCCEEDED(hr)) {
        IWbemServices* services = nullptr;
        hr = locator->ConnectServer(
            _bstr_t(L"ROOT\\CIMV2"),
            nullptr, nullptr, nullptr, 0, nullptr, nullptr,
            &services);
        
        if (SUCCEEDED(hr)) {
            // Set security levels
            CoSetProxyBlanket(services, RPC_C_AUTHN_WINNT, RPC_C_AUTHZ_NONE,
                            nullptr, RPC_C_AUTHN_LEVEL_CALL,
                            RPC_C_IMP_LEVEL_IMPERSONATE, nullptr, EOAC_NONE);
            
            IEnumWbemClassObject* enumerator = nullptr;
            hr = services->ExecQuery(
                _bstr_t(L"WQL"),
                _bstr_t(L"SELECT * FROM Win32_PhysicalMemory"),
                WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,
                nullptr, &enumerator);
            
            if (SUCCEEDED(hr)) {
                IWbemClassObject* obj = nullptr;
                ULONG returned = 0;
                
                while (enumerator->Next(WBEM_INFINITE, 1, &obj, &returned) == S_OK) {
                    MemoryInfo::MemoryModule module;
                    
                    VARIANT var;
                    
                    // Get manufacturer
                    VariantInit(&var);
                    if (SUCCEEDED(obj->Get(L"Manufacturer", 0, &var, nullptr, nullptr))) {
                        if (var.vt == VT_BSTR) {
                            module.manufacturer = BSTRToString(var.bstrVal);
                        }
                        VariantClear(&var);
                    }
                    
                    // Get part number
                    VariantInit(&var);
                    if (SUCCEEDED(obj->Get(L"PartNumber", 0, &var, nullptr, nullptr))) {
                        if (var.vt == VT_BSTR) {
                            module.part_number = BSTRToString(var.bstrVal);
                        }
                        VariantClear(&var);
                    }
                    
                    // Get capacity
                    VariantInit(&var);
                    if (SUCCEEDED(obj->Get(L"Capacity", 0, &var, nullptr, nullptr))) {
                        if (var.vt == VT_BSTR) {
                            module.capacity_bytes = _wcstoui64(var.bstrVal, nullptr, 10);
                        } else if (var.vt == VT_UI8) {
                            module.capacity_bytes = var.ullVal;
                        }
                        VariantClear(&var);
                    }
                    
                    // Get speed
                    VariantInit(&var);
                    if (SUCCEEDED(obj->Get(L"Speed", 0, &var, nullptr, nullptr))) {
                        if (var.vt == VT_I4) {
                            module.speed_mhz = var.lVal;
                        }
                        VariantClear(&var);
                    }
                    
                    // Get memory type
                    VariantInit(&var);
                    if (SUCCEEDED(obj->Get(L"SMBIOSMemoryType", 0, &var, nullptr, nullptr))) {
                        if (var.vt == VT_I4) {
                            switch (var.lVal) {
                                case 20: module.type = "DDR"; break;
                                case 21: module.type = "DDR2"; break;
                                case 22: module.type = "DDR2 FB-DIMM"; break;
                                case 24: module.type = "DDR3"; break;
                                case 26: module.type = "DDR4"; break;
                                case 34: module.type = "DDR5"; break;
                                default: module.type = "Unknown";
                            }
                        }
                        VariantClear(&var);
                    }
                    
                    modules.push_back(module);
                    obj->Release();
                }
                
                enumerator->Release();
            }
            
            services->Release();
        }
        
        locator->Release();
    }
    
    CoUninitialize();
    
    return modules;
}

void MemoryMonitor::query_smbios_memory() {
    // Alternative SMBIOS query implementation
    // This method queries the system firmware tables directly
    // for memory information without using WMI
    
    DWORD smbios_signature = 0x524D5342; // 'RSMB'
    DWORD smbios_size = GetSystemFirmwareTable(smbios_signature, 0, nullptr, 0);
    if (smbios_size == 0) return;
    
    std::vector<BYTE> smbios_data(smbios_size);
    if (GetSystemFirmwareTable(smbios_signature, 0, smbios_data.data(), smbios_size) == 0) {
        return;
    }
    
    // Parse SMBIOS data structures
    // This is a simplified version - full parsing would be more complex
    BYTE* data = smbios_data.data();
    DWORD offset = 8;  // Skip SMBIOS header
    
    while (offset < smbios_size) {
        BYTE type = data[offset];
        BYTE length = data[offset + 1];
        
        if (type == 17) {  // Memory Device
            // Parse memory device structure
            // Implementation details would go here
        }
        
        if (type == 127) break;  // End of table
        
        // Move to next structure
        offset += length;
        while (offset < smbios_size && (data[offset] != 0 || data[offset + 1] != 0)) {
            offset++;
        }
        offset += 2;  // Skip double null terminator
    }
}

std::string MemoryInfo::to_json() const {
    std::ostringstream oss;
    oss << "{"
        << "\"total_physical_bytes\":" << total_physical_bytes << ","
        << "\"available_physical_bytes\":" << available_physical_bytes << ","
        << "\"used_physical_bytes\":" << used_physical_bytes << ","
        << "\"usage_percent\":" << std::fixed << std::setprecision(2) << usage_percent << ","
        << "\"total_page_file_bytes\":" << total_page_file_bytes << ","
        << "\"available_page_file_bytes\":" << available_page_file_bytes << ","
        << "\"total_virtual_bytes\":" << total_virtual_bytes << ","
        << "\"available_virtual_bytes\":" << available_virtual_bytes << ","
        << "\"modules\":[";
    
    for (size_t i = 0; i < modules.size(); ++i) {
        if (i > 0) oss << ",";
        oss << "{"
            << "\"manufacturer\":\"" << modules[i].manufacturer << "\","
            << "\"part_number\":\"" << modules[i].part_number << "\","
            << "\"capacity_bytes\":" << modules[i].capacity_bytes << ","
            << "\"speed_mhz\":" << modules[i].speed_mhz << ","
            << "\"type\":\"" << modules[i].type << "\""
            << "}";
    }
    oss << "]";
    
    oss << "}";
    return oss.str();
}

} // namespace smn