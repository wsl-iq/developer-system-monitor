/**
 * @file cpu.cpp
 * @brief CPU monitoring and information retrieval.
 * @addtogroup CPU
 * Copyright © 2026, inc. Mohammed Al-Baqer. All rights reserved.
 */

#include "cpu.hpp"
#include <windows.h>
#include <pdh.h>
#include <pdhmsg.h>
#include <string>
#include <sstream>
#include <iomanip>
#include <vector>
#include <bitset>
#include <thread>
#include <chrono>
#include <comdef.h>
#include <wbemidl.h>
#include <algorithm>

#pragma comment(lib, "pdh.lib")
#pragma comment(lib, "wbemuuid.lib")
#pragma comment(lib, "ole32.lib")
#pragma comment(lib, "oleaut32.lib")
#pragma comment(lib, "advapi32.lib")

namespace smn {

// Performance counter helper class
class PDHQuery {
private:
    PDH_HQUERY query_;
    std::vector<PDH_HCOUNTER> counters_;
    bool initialized_;
    
public:
    PDHQuery() : query_(nullptr), initialized_(false) {}
    
    ~PDHQuery() {
        close();
    }
    
    bool initialize() {
        if (initialized_) return true;
        
        PDH_STATUS status = PdhOpenQueryW(nullptr, 0, &query_);
        if (status == ERROR_SUCCESS) {
            initialized_ = true;
            return true;
        }
        return false;
    }
    
    PDH_HCOUNTER add_counter(const std::wstring& path) {
        if (!initialized_) return nullptr;
        
        PDH_HCOUNTER counter = nullptr;
        PDH_STATUS status = PdhAddCounterW(query_, path.c_str(), 0, &counter);
        if (status == ERROR_SUCCESS) {
            counters_.push_back(counter);
            return counter;
        }
        return nullptr;
    }
    
    PDH_HCOUNTER add_english_counter(const std::wstring& path) {
        if (!initialized_) return nullptr;
        
        PDH_HCOUNTER counter = nullptr;
        PDH_STATUS status = PdhAddEnglishCounterW(query_, path.c_str(), 0, &counter);
        if (status == ERROR_SUCCESS) {
            counters_.push_back(counter);
            return counter;
        }
        return nullptr;
    }
    
    bool collect_data() {
        if (!initialized_) return false;
        
        PDH_STATUS status = PdhCollectQueryData(query_);
        return status == ERROR_SUCCESS;
    }
    
    double get_counter_value(PDH_HCOUNTER counter) {
        if (!counter) return 0.0;
        
        PDH_FMT_COUNTERVALUE value;
        PDH_STATUS status = PdhGetFormattedCounterValue(counter, PDH_FMT_DOUBLE, 
                                                         nullptr, &value);
        if (status == ERROR_SUCCESS) {
            return value.doubleValue;
        }
        return 0.0;
    }
    
    long get_counter_value_long(PDH_HCOUNTER counter) {
        if (!counter) return 0;
        
        PDH_FMT_COUNTERVALUE value;
        PDH_STATUS status = PdhGetFormattedCounterValue(counter, PDH_FMT_LONG, nullptr, &value);
        if (status == ERROR_SUCCESS) {
            return value.longValue;
        }
        return 0;
    }
    
    void close() {
        if (query_) {
            PdhCloseQuery(query_);
            query_ = nullptr;
        }
        counters_.clear();
        initialized_ = false;
    }
};

CPUMonitor::CPUMonitor() 
    : pdh_query_(nullptr)
    , cpu_counter_(nullptr)
    , frequency_counter_(nullptr) {
    initialize_counters();
}

CPUMonitor::~CPUMonitor() {
    cleanup_counters();
}

void CPUMonitor::initialize_counters() {
    // Initialize performance query
    pdh_query_ = new PDHQuery();
    if (!static_cast<PDHQuery*>(pdh_query_)->initialize()) {
        delete static_cast<PDHQuery*>(pdh_query_);
        pdh_query_ = nullptr;
        return;
    }
    
    // Add CPU counters
    cpu_counter_ = static_cast<PDHQuery*>(pdh_query_)->add_english_counter(
        L"\\Processor(_Total)\\% Processor Time");
    
    // Add per-core counters
    SYSTEM_INFO sys_info;
    GetSystemInfo(&sys_info);
    
    for (DWORD i = 0; i < sys_info.dwNumberOfProcessors; ++i) {
        std::wstringstream counter_path;
        counter_path << L"\\Processor(" << i << L")\\% Processor Time";
        
        PDH_HCOUNTER counter = static_cast<PDHQuery*>(pdh_query_)->add_english_counter(
            counter_path.str());
        if (counter) {
            core_counters_.push_back(counter);
        }
    }
    
    // Add frequency counter
    frequency_counter_ = static_cast<PDHQuery*>(pdh_query_)->add_english_counter(
        L"\\Processor Information(_Total)\\% Processor Frequency");
    
    // Initial data collection
    static_cast<PDHQuery*>(pdh_query_)->collect_data();
}

void CPUMonitor::cleanup_counters() {
    if (pdh_query_) {
        delete static_cast<PDHQuery*>(pdh_query_);
        pdh_query_ = nullptr;
    }
    cpu_counter_ = nullptr;
    frequency_counter_ = nullptr;
    core_counters_.clear();
}

CPUInfo CPUMonitor::get_cpu_info() {
    CPUInfo info;
    
    // Get basic CPU information
    info.manufacturer = get_cpu_manufacturer();
    info.model = get_cpu_model();
    info.physical_cores = get_core_count();
    info.logical_processors = get_logical_processor_count();
    info.current_frequency_mhz = get_current_frequency();
    info.max_frequency_mhz = get_max_frequency();
    
    // Get real-time metrics
    info.total_usage_percent = get_total_usage_percent();
    info.core_usages = get_per_core_usage();
    
    // Try to get temperature via WMI
    info.temperature_celsius = get_temperature();
    info.temperature_available = (info.temperature_celsius > 0);
    
    // Get architecture
    SYSTEM_INFO sys_info;
    GetNativeSystemInfo(&sys_info);
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

double CPUMonitor::get_total_usage_percent() {
    if (!pdh_query_ || !cpu_counter_) return 0.0;
    
    PDHQuery* query = static_cast<PDHQuery*>(pdh_query_);
    
    // Collect data
    query->collect_data();
    Sleep(100);  // Wait for sample
    query->collect_data();
    
    return query->get_counter_value(cpu_counter_);
}

std::vector<double> CPUMonitor::get_per_core_usage() {
    std::vector<double> usages;
    
    if (!pdh_query_) return usages;
    
    PDHQuery* query = static_cast<PDHQuery*>(pdh_query_);
    
    // Collect data
    query->collect_data();
    Sleep(100);
    query->collect_data();
    
    for (auto counter : core_counters_) {
        usages.push_back(query->get_counter_value(counter));
    }
    
    return usages;
}

uint32_t CPUMonitor::get_current_frequency() {
    uint32_t frequency = 0;
    
    // Try performance counter first
    if (pdh_query_ && frequency_counter_) {
        PDHQuery* query = static_cast<PDHQuery*>(pdh_query_);
        query->collect_data();
        Sleep(50);
        query->collect_data();
        
        double freq_mhz = query->get_counter_value(frequency_counter_);
        if (freq_mhz > 0) {
            return static_cast<uint32_t>(freq_mhz);
        }
    }
    
    // Fallback to registry
    HKEY hKey;
    if (RegOpenKeyExA(HKEY_LOCAL_MACHINE, 
                      "HARDWARE\\DESCRIPTION\\System\\CentralProcessor\\0",
                      0, KEY_READ, &hKey) == ERROR_SUCCESS) {
        DWORD mhz;
        DWORD size = sizeof(mhz);
        if (RegQueryValueExA(hKey, "~MHz", nullptr, nullptr,
                            (LPBYTE)&mhz, &size) == ERROR_SUCCESS) {
            frequency = mhz;
        }
        RegCloseKey(hKey);
    }
    
    return frequency;
}

uint32_t CPUMonitor::get_max_frequency() {
    uint32_t max_frequency = 0;
    
    HKEY hKey;
    if (RegOpenKeyExA(HKEY_LOCAL_MACHINE, 
                      "HARDWARE\\DESCRIPTION\\System\\CentralProcessor\\0",
                      0, KEY_READ, &hKey) == ERROR_SUCCESS) {
        DWORD mhz;
        DWORD size = sizeof(mhz);
        if (RegQueryValueExA(hKey, "~MHz", nullptr, nullptr,
                            (LPBYTE)&mhz, &size) == ERROR_SUCCESS) {
            max_frequency = mhz;
        }
        RegCloseKey(hKey);
    }
    
    return max_frequency;
}

double CPUMonitor::get_temperature() {
    double temperature = 0.0;
    
    // Query WMI for CPU temperature
    HRESULT hr = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
    if (FAILED(hr)) return 0.0;
    
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
                _bstr_t(L"SELECT * FROM Win32_PerfFormattedData_Counters_ThermalZoneInformation"),
                WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,
                nullptr, &enumerator);
            
            if (SUCCEEDED(hr)) {
                IWbemClassObject* obj = nullptr;
                ULONG returned = 0;
                
                while (enumerator->Next(WBEM_INFINITE, 1, &obj, &returned) == S_OK) {
                    VARIANT var;
                    VariantInit(&var);
                    
                    if (SUCCEEDED(obj->Get(L"Temperature", 0, &var, nullptr, nullptr))) {
                        if (var.vt == VT_I4 || var.vt == VT_UI4) {
                            // Temperature is in tenths of Kelvin
                            double temp_kelvin = var.lVal / 10.0;
                            temperature = temp_kelvin - 273.15;  // Convert to Celsius
                        }
                        VariantClear(&var);
                    }
                    
                    obj->Release();
                }
                
                enumerator->Release();
            }
            
            services->Release();
        }
        
        locator->Release();
    }
    
    CoUninitialize();
    
    return temperature;
}

std::string CPUMonitor::get_cpu_manufacturer() {
    return query_registry_string(
        "HARDWARE\\DESCRIPTION\\System\\CentralProcessor\\0",
        "VendorIdentifier");
}

std::string CPUMonitor::get_cpu_model() {
    return query_registry_string(
        "HARDWARE\\DESCRIPTION\\System\\CentralProcessor\\0",
        "ProcessorNameString");
}

uint32_t CPUMonitor::get_core_count() {
    DWORD length = 0;
    GetLogicalProcessorInformation(nullptr, &length);
    
    if (length == 0) {
        SYSTEM_INFO sys_info;
        GetSystemInfo(&sys_info);
        return sys_info.dwNumberOfProcessors;
    }
    
    std::vector<SYSTEM_LOGICAL_PROCESSOR_INFORMATION> buffer(
        length / sizeof(SYSTEM_LOGICAL_PROCESSOR_INFORMATION));
    
    if (GetLogicalProcessorInformation(buffer.data(), &length)) {
        uint32_t physical_cores = 0;
        for (const auto& info : buffer) {
            if (info.Relationship == RelationProcessorCore) {
                physical_cores++;
            }
        }
        return physical_cores;
    }
    
    SYSTEM_INFO sys_info;
    GetSystemInfo(&sys_info);
    return sys_info.dwNumberOfProcessors;
}

uint32_t CPUMonitor::get_logical_processor_count() {
    DWORD length = 0;
    GetLogicalProcessorInformationEx(RelationProcessorCore, nullptr, &length);
    
    if (length == 0) {
        SYSTEM_INFO sys_info;
        GetSystemInfo(&sys_info);
        return sys_info.dwNumberOfProcessors;
    }
    
    std::vector<BYTE> buffer(length);
    
    if (GetLogicalProcessorInformationEx(RelationProcessorCore, 
                                        (PSYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX)buffer.data(),
                                        &length)) {
        uint32_t logical_count = 0;
        PSYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX info = 
            (PSYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX)buffer.data();
        
        DWORD offset = 0;
        while (offset < length) {
            info = (PSYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX)(buffer.data() + offset);
            
            if (info->Relationship == RelationProcessorCore) {
                // Count set bits in processor mask
                for (int i = 0; i < info->Processor.GroupCount; ++i) {
                    logical_count += std::bitset<64>(info->Processor.GroupMask[i].Mask).count();
                }
            }
            
            offset += info->Size;
        }
        
        return logical_count;
    }
    
    SYSTEM_INFO sys_info;
    GetSystemInfo(&sys_info);
    return sys_info.dwNumberOfProcessors;
}

std::string CPUMonitor::query_registry_string(const std::string& key_path, 
                                               const std::string& value_name) {
    std::string result;
    HKEY hKey;
    
    if (RegOpenKeyExA(HKEY_LOCAL_MACHINE, key_path.c_str(), 0, 
                      KEY_READ, &hKey) == ERROR_SUCCESS) {
        char value[512];
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

std::string CPUInfo::to_json() const {
    std::ostringstream oss;
    oss << "{"
        << "\"manufacturer\":\"" << manufacturer << "\","
        << "\"model\":\"" << model << "\","
        << "\"architecture\":\"" << architecture << "\","
        << "\"physical_cores\":" << physical_cores << ","
        << "\"logical_processors\":" << logical_processors << ","
        << "\"current_frequency_mhz\":" << current_frequency_mhz << ","
        << "\"max_frequency_mhz\":" << max_frequency_mhz << ","
        << "\"total_usage_percent\":" << std::fixed << std::setprecision(2) 
        << total_usage_percent << ","
        << "\"core_usages\":[";
    
    for (size_t i = 0; i < core_usages.size(); ++i) {
        if (i > 0) oss << ",";
        oss << std::fixed << std::setprecision(2) << core_usages[i];
    }
    oss << "]";
    
    if (temperature_available) {
        oss << ",\"temperature_celsius\":" << std::fixed << std::setprecision(1) 
            << temperature_celsius;
    }
    
    oss << "}";
    return oss.str();
}

} // namespace smn