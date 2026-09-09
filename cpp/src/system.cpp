/**
 * @file system.cpp
 * @brief System monitoring and information retrieval.
 * @addtogroup System
 * Copyright © 2026, inc. Mohammed Al-Baqer. All rights reserved.
 */

#include "system.hpp"
#include <windows.h>
#include <sstream>
#include <iomanip>
#include <cstring>
#include <algorithm>
#include <cctype>
#include <cstdio>

namespace smn {

namespace {

std::string json_escape(const std::string& str) {
    std::string result;
    result.reserve(str.length() * 2);

    for (unsigned char c : str) {
        switch (c) {
            case '"': result += "\\\""; break;
            case '\\': result += "\\\\"; break;
            case '\n': result += "\\n"; break;
            case '\r': result += "\\r"; break;
            case '\t': result += "\\t"; break;
            case '\b': result += "\\b"; break;
            case '\f': result += "\\f"; break;
            default:
                if (c < 0x20) {
                    char buf[8];
                    std::snprintf(buf, sizeof(buf), "\\u%04x", c);
                    result += buf;
                } else {
                    result += static_cast<char>(c);
                }
        }
    }

    return result;
}

std::string sanitize_registry_value(const std::string& value) {
    std::string cleaned;
    cleaned.reserve(value.size());

    for (unsigned char c : value) {
        if (c == '\0' || c == '\n' || c == '\r' || c == '\t') {
            continue;
        }

        if (std::isprint(c) != 0) {
            if (std::isalnum(c) || c == '.' || c == '-' || c == '_' || c == ' ' ||
                c == '(' || c == ')' || c == '/' || c == ':' || c == ';' || c == '+' ||
                c == '[' || c == ']' || c == '{' || c == '}' || c == ',' || c == '@' || c == '#') {
                cleaned.push_back(static_cast<char>(c));
            }
        }
    }

    auto begin = cleaned.find_first_not_of(" \t\r\n");
    if (begin == std::string::npos) {
        return "";
    }

    auto end = cleaned.find_last_not_of(" \t\r\n");
    return cleaned.substr(begin, end - begin + 1);
}

std::string sanitize_version_value(const std::string& value) {
    std::string sanitized = sanitize_registry_value(value);
    if (sanitized.empty()) {
        return "10.0";
    }
    return sanitized;
}

} // namespace

SystemMonitor::SystemMonitor() 
    : initialized_(false), 
      cpu_counter_(nullptr), 
      memory_counter_(nullptr) {
    initialize_counters();
}

SystemMonitor::~SystemMonitor() {
    cleanup_counters();
}

void SystemMonitor::initialize_counters() {
    initialized_ = true;
}

void SystemMonitor::cleanup_counters() {
    cpu_counter_ = nullptr;
    memory_counter_ = nullptr;
    initialized_ = false;
}

SystemInfo SystemMonitor::get_system_info() {
    SystemInfo info;
    
    // Initialize with default values
    info.hostname = "Unknown";         // Default hostname
    info.os_name = "Windows";         // Default OS name
    info.os_version = "10.0";        // Default OS version
    info.os_build = "0";            // Default OS build
    info.architecture = "Unknown"; // Default architecture
    info.uptime_seconds = 0;      // Default uptime
    info.processor_count = 0;    // Default processor count
    
    // Get hostname
    char hostname_buffer[256] = {0};
    DWORD hostname_size = sizeof(hostname_buffer);
    if (GetComputerNameA(hostname_buffer, &hostname_size) && hostname_buffer[0] != '\0') {
        info.hostname = sanitize_registry_value(hostname_buffer);
        if (info.hostname.empty()) {
            info.hostname = "Unknown";
        }
    }
    
    // Get OS information from registry
    HKEY hKey = nullptr;
    LONG reg_result = RegOpenKeyExA(
        HKEY_LOCAL_MACHINE, 
        "SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion",
        0, 
        KEY_READ,
        &hKey
    );
    
    if (reg_result == ERROR_SUCCESS && hKey != nullptr) {
        char buffer[512] = {0};
        DWORD buffer_size = sizeof(buffer);
        DWORD type = 0;
        
        // Get ProductName
        memset(buffer, 0, sizeof(buffer));
        buffer_size = sizeof(buffer);
        if (RegQueryValueExA(hKey, "ProductName", nullptr, &type,
                            (LPBYTE)buffer, &buffer_size) == ERROR_SUCCESS 
            && buffer[0] != '\0') {
            info.os_name = sanitize_registry_value(buffer);
            if (info.os_name.empty()) {
                info.os_name = "Windows";
            }
        }
        
        // Get CurrentVersion (this exists on all Windows versions)
        memset(buffer, 0, sizeof(buffer));
        buffer_size = sizeof(buffer);
        if (RegQueryValueExA(hKey, "CurrentVersion", nullptr, &type,
                            (LPBYTE)buffer, &buffer_size) == ERROR_SUCCESS 
            && buffer[0] != '\0') {
            info.os_version = sanitize_version_value(buffer);
        }
        
        // Try to get more specific version from CurrentMajorVersionNumber
        memset(buffer, 0, sizeof(buffer));
        buffer_size = sizeof(buffer);
        if (RegQueryValueExA(hKey, "CurrentMajorVersionNumber", nullptr, &type,
                            (LPBYTE)buffer, &buffer_size) == ERROR_SUCCESS 
            && buffer[0] != '\0') {
            std::string major = sanitize_version_value(buffer);
            
            memset(buffer, 0, sizeof(buffer));
            buffer_size = sizeof(buffer);
            if (RegQueryValueExA(hKey, "CurrentMinorVersionNumber", nullptr, &type,
                                (LPBYTE)buffer, &buffer_size) == ERROR_SUCCESS 
                && buffer[0] != '\0') {
                std::string minor = sanitize_version_value(buffer);
                if (!minor.empty()) {
                    info.os_version = major + "." + minor;
                } else {
                    info.os_version = major;
                }
            } else {
                info.os_version = major;
            }
        }
        
        // Get CurrentBuildNumber
        memset(buffer, 0, sizeof(buffer));
        buffer_size = sizeof(buffer);
        if (RegQueryValueExA(hKey, "CurrentBuildNumber", nullptr, &type,
                            (LPBYTE)buffer, &buffer_size) == ERROR_SUCCESS 
            && buffer[0] != '\0') {
            info.os_build = sanitize_registry_value(buffer);
            if (info.os_build.empty()) {
                info.os_build = "0";
            }
        }
        
        RegCloseKey(hKey);
    }
    
    // Get architecture and processor count
    SYSTEM_INFO sys_info;
    memset(&sys_info, 0, sizeof(sys_info));
    GetNativeSystemInfo(&sys_info);
    
    switch (sys_info.wProcessorArchitecture) { 
        case PROCESSOR_ARCHITECTURE_AMD64:           // x64 architecture
            info.architecture = "x64";              // Default architecture
            break;                                 // ARM architecture
        case PROCESSOR_ARCHITECTURE_INTEL:        // x86 architecture
            info.architecture = "x86";           // Default architecture
            break;                              // ARM architecture
        case PROCESSOR_ARCHITECTURE_ARM64:     // ARM64 architecture
            info.architecture = "ARM64";      // Default architecture
            break;                           // ARM architecture
        default: 
            info.architecture = "Unknown"; // Default architecture
    }
    
    info.processor_count = sys_info.dwNumberOfProcessors;
    info.uptime_seconds = GetTickCount64() / 1000;
    
    return info;
}

uint64_t SystemMonitor::get_system_uptime() {
    return GetTickCount64() / 1000;
}

std::string SystemMonitor::get_os_name() {
    HKEY hKey = nullptr;
    std::string os_name = "Windows";
    if (RegOpenKeyExA(HKEY_LOCAL_MACHINE,
                      "SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion",
                      0, KEY_READ, &hKey) == ERROR_SUCCESS && hKey != nullptr) {
        char buffer[256] = {0};
        DWORD size = sizeof(buffer);
        DWORD type = 0;

        if (RegQueryValueExA(hKey, "ProductName", nullptr, &type,
                            (LPBYTE)buffer, &size) == ERROR_SUCCESS 
            && buffer[0] != '\0') {
            os_name = sanitize_registry_value(buffer);
            if (os_name.empty()) {
                os_name = "Windows";
            }
        }

        RegCloseKey(hKey);
    }

    return os_name;
}

std::string SystemMonitor::get_os_version() {
    std::string version = "10.0";
    
    HKEY hKey = nullptr;
    if (RegOpenKeyExA(HKEY_LOCAL_MACHINE,
                      "SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion",
                      0, KEY_READ, &hKey) == ERROR_SUCCESS && hKey != nullptr) {
        char buffer[32] = {0};
        DWORD size = sizeof(buffer);
        DWORD type = 0;

        // Try to get the version from CurrentMajorVersionNumber and CurrentMinorVersionNumber
        // If those values are not available, fallback to CurrentVersion
        // This ensures compatibility with older Windows versions

        // First, try to get CurrentMajorVersionNumber
        // If it exists, we will also check for CurrentMinorVersionNumber
        memset(buffer, 0, sizeof(buffer));
        size = sizeof(buffer);
        // Check if CurrentMajorVersionNumber exists
        // If it exists, we will also check for CurrentMinorVersionNumber
        // If both exist, we will construct the version string as "major.minor"



        if (RegQueryValueExA(hKey, "CurrentMajorVersionNumber", nullptr, &type,
                            (LPBYTE)buffer, &size) == ERROR_SUCCESS  // Check if CurrentMajorVersionNumber exists
                            // If it exists, we will also check for CurrentMinorVersionNumber
            && buffer[0] != '\0') {
            std::string major = sanitize_version_value(buffer);  // Get the major version number

            memset(buffer, 0, sizeof(buffer));
            size = sizeof(buffer);
            if (RegQueryValueExA(hKey, "CurrentMinorVersionNumber", nullptr, &type,
                                (LPBYTE)buffer, &size) == ERROR_SUCCESS // Check if CurrentMinorVersionNumber exists
                && buffer[0] != '\0') {
                std::string minor = sanitize_version_value(buffer);
                if (!minor.empty()) {
                    version = major + "." + minor;
                } else {
                    version = major;
                }
            } else {
                version = major;
            }
        } else {
            memset(buffer, 0, sizeof(buffer));
            size = sizeof(buffer);
            if (RegQueryValueExA(hKey, "CurrentVersion", nullptr, &type,
                                (LPBYTE)buffer, &size) == ERROR_SUCCESS 
                && buffer[0] != '\0') {
                version = sanitize_version_value(buffer);
            }
        }

        RegCloseKey(hKey);
    }

    return version;
}

std::string SystemMonitor::get_os_build() {
    std::string build = "0"; // Default build number


    // Get the build number from the registry
    // This is a fallback in case the build number is not available from other sources
    // We will query the registry key "SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion" for the value "CurrentBuildNumber"
    // If the value is not available, we will return the default build number "0"
    HKEY hKey = nullptr;
    if (RegOpenKeyExA(HKEY_LOCAL_MACHINE,
                      "SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion",
                      0, KEY_READ, &hKey) == ERROR_SUCCESS && hKey != nullptr) {
        char buffer[64] = {0};
        DWORD size = sizeof(buffer);
        DWORD type = 0;
        
        if (RegQueryValueExA(hKey, "CurrentBuildNumber", nullptr, &type,
                            (LPBYTE)buffer, &size) == ERROR_SUCCESS 
            && buffer[0] != '\0') {
            build = sanitize_registry_value(buffer);
            if (build.empty()) {
                build = "0";
            }
        }
        
        RegCloseKey(hKey);
    }
    
    return build;
}

/**
 * CPU and Memory metrics retrieval functions.
 * These functions are placeholders and should be implemented to retrieve actual metrics.
 * metrics.total_usage_percent, metrics.per_core_usage, metrics.frequency_mhz, metrics.base_frequency_mhz, and metrics.temperature_celsius should be populated with real data.
 * metrics.temperature_available should be set to true if temperature data is available, false otherwise.
 */
CPUMetrics SystemMonitor::get_cpu_metrics() {
    CPUMetrics metrics;
    metrics.total_usage_percent = 0;
    metrics.frequency_mhz = 0;
    metrics.base_frequency_mhz = 0;
    metrics.temperature_celsius = 0;
    metrics.temperature_available = false;
    return metrics;
}

// Memory metrics retrieval function. This function is a placeholder and should be implemented to retrieve actual memory metrics.
// metrics.total_physical_bytes, metrics.available_physical_bytes, 
// metrics.used_physical_bytes, 
// metrics.usage_percent,
// metrics.total_page_file_bytes,
// metrics.available_page_file_bytes,
//  metrics.total_virtual_bytes, and metrics.available_virtual_bytes should be populated with real data. 
MemoryMetrics SystemMonitor::get_memory_metrics() {
    MemoryMetrics metrics;
    metrics.total_physical_bytes = 0;
    metrics.available_physical_bytes = 0;
    metrics.used_physical_bytes = 0;
    metrics.usage_percent = 0;
    metrics.total_page_file_bytes = 0;
    metrics.available_page_file_bytes = 0;
    metrics.total_virtual_bytes = 0;
    metrics.available_virtual_bytes = 0;
    return metrics;
}

std::string SystemMonitor::get_all_metrics_json() {
    return "{}";
}

std::string SystemInfo::to_json() const {
    std::ostringstream oss;
    oss << "{"
        << "\"hostname\":\"" << json_escape(hostname) << "\","
        << "\"os_name\":\"" << json_escape(os_name) << "\","
        << "\"os_version\":\"" << json_escape(os_version) << "\","
        << "\"os_build\":\"" << json_escape(os_build) << "\","
        << "\"architecture\":\"" << json_escape(architecture) << "\","
        << "\"uptime_seconds\":" << uptime_seconds << ","
        << "\"processor_count\":" << processor_count
        << "}";
    return oss.str();
}

std::string CPUMetrics::to_json() const {
    std::ostringstream oss;
    oss << "{"
        << "\"total_usage_percent\":" << total_usage_percent << ","
        << "\"frequency_mhz\":" << frequency_mhz << ","
        << "\"base_frequency_mhz\":" << base_frequency_mhz;
    
    if (temperature_available) {
        oss << ",\"temperature_celsius\":" << temperature_celsius;
    }
    
    oss << ",\"per_core_usage\":[";
    for (size_t i = 0; i < per_core_usage.size(); ++i) {
        if (i > 0) oss << ",";
        oss << per_core_usage[i];
    }
    oss << "]";
    
    oss << "}";
    return oss.str();
}

std::string MemoryMetrics::to_json() const {
    std::ostringstream oss;
    oss << "{"
        << "\"total_physical_bytes\":" << total_physical_bytes << ","
        << "\"available_physical_bytes\":" << available_physical_bytes << ","
        << "\"used_physical_bytes\":" << used_physical_bytes << ","
        << "\"usage_percent\":" << usage_percent << ","
        << "\"total_page_file_bytes\":" << total_page_file_bytes << ","
        << "\"available_page_file_bytes\":" << available_page_file_bytes << ","
        << "\"total_virtual_bytes\":" << total_virtual_bytes << ","
        << "\"available_virtual_bytes\":" << available_virtual_bytes
        << "}";
    return oss.str();
}

} // namespace smn