/**
 * @file services.cpp
 * @brief Service monitoring and information retrieval.
 * @addtogroup Services
 * Copyright © 2026, inc. Mohammed Al-Baqer. All rights reserved.
 */

#include "services.hpp"
#include <windows.h>
#include <winsvc.h>
#include <sstream>
#include <iomanip>
#include <cstdio>
#include <cstring>

#pragma comment(lib, "advapi32.lib")

namespace smn {

// JSON escape helper
static std::string json_escape(const std::string& str) {
    std::string result;
    result.reserve(str.length() * 2);

    for (unsigned char c : str) {
        switch (c) {
            case '"': result += "\\\""; break;
            case '\\': result += "\\\\"; break;
            case '\n': result += "\\n"; break;
            case '\r': result += "\\r"; break;
            case '\t': result += "\\t"; break;
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

ServiceManager::ServiceManager() : service_manager_handle_(nullptr) {
    initialize();
}

ServiceManager::~ServiceManager() {
    cleanup();
}

void ServiceManager::initialize() {
    service_manager_handle_ = OpenSCManagerA(nullptr, nullptr, SC_MANAGER_ENUMERATE_SERVICE);
}

void ServiceManager::cleanup() {
    if (service_manager_handle_) {
        CloseServiceHandle(service_manager_handle_);
        service_manager_handle_ = nullptr;
    }
}

std::vector<ServiceInfo> ServiceManager::get_service_list() {
    std::vector<ServiceInfo> services;
    
    if (!service_manager_handle_) {
        initialize();
    }

    if (!service_manager_handle_) {
        return services;
    }
    
    DWORD bytes_needed = 0;
    DWORD services_count = 0;
    DWORD resume_handle = 0;
    
    EnumServicesStatusExA(service_manager_handle_, SC_ENUM_PROCESS_INFO,
                         SERVICE_WIN32, SERVICE_STATE_ALL, nullptr, 0,
                         &bytes_needed, &services_count, &resume_handle, nullptr);
    
    if (bytes_needed == 0) {
        return services;
    }
    
    std::vector<BYTE> buffer(bytes_needed + 1024);
    LPENUM_SERVICE_STATUS_PROCESSA service_status = 
        (LPENUM_SERVICE_STATUS_PROCESSA)buffer.data();
    
    if (EnumServicesStatusExA(service_manager_handle_, SC_ENUM_PROCESS_INFO,
                             SERVICE_WIN32, SERVICE_STATE_ALL, buffer.data(),
                             bytes_needed + 1024, &bytes_needed, &services_count,
                             &resume_handle, nullptr)) {
        for (DWORD i = 0; i < services_count; ++i) {
            ServiceInfo info;
            
            if (service_status[i].lpServiceName) {
                info.service_name = service_status[i].lpServiceName;
            }
            if (service_status[i].lpDisplayName) {
                info.display_name = service_status[i].lpDisplayName;
            }
            
            info.status = get_service_status_string(
                service_status[i].ServiceStatusProcess.dwCurrentState);
            
            info.process_id = service_status[i].ServiceStatusProcess.dwProcessId;
            
            // Get startup type
            SC_HANDLE service = OpenServiceA(service_manager_handle_, 
                                            info.service_name.c_str(), 
                                            SERVICE_QUERY_CONFIG);
            if (service) {
                DWORD config_bytes_needed = 0;
                QueryServiceConfigA(service, nullptr, 0, &config_bytes_needed);
                if (config_bytes_needed > 0) {
                    std::vector<BYTE> config_buffer(config_bytes_needed + 256);
                    LPQUERY_SERVICE_CONFIGA config = 
                        (LPQUERY_SERVICE_CONFIGA)config_buffer.data();
                    if (QueryServiceConfigA(service, config, config_bytes_needed + 256, 
                                           &config_bytes_needed)) {
                        info.startup_type = get_service_start_type_string(
                            config->dwStartType);
                        if (config->lpBinaryPathName) {
                            info.binary_path = config->lpBinaryPathName;
                        }
                    }
                }
                CloseServiceHandle(service);
            }
            
            services.push_back(info);
        }
    }
    
    return services;
}

ServiceInfo ServiceManager::get_service_info(const std::string& service_name) {
    ServiceInfo info;
    info.service_name = service_name;
    
    if (!service_manager_handle_) {
        return info;
    }
    
    SC_HANDLE service = OpenServiceA(service_manager_handle_, 
                                    service_name.c_str(), SERVICE_QUERY_STATUS);
    if (service) {
        SERVICE_STATUS_PROCESS status;
        DWORD bytes_needed;
        if (QueryServiceStatusEx(service, SC_STATUS_PROCESS_INFO,
                                (LPBYTE)&status, sizeof(status), &bytes_needed)) {
            info.status = get_service_status_string(status.dwCurrentState);
            info.process_id = status.dwProcessId;
        }
        CloseServiceHandle(service);
    }
    
    return info;
}

bool ServiceManager::start_service(const std::string& service_name) {
    if (!service_manager_handle_) {
        return false;
    }
    
    SC_HANDLE service = OpenServiceA(service_manager_handle_, 
                                    service_name.c_str(), SERVICE_START);
    if (!service) {
        return false;
    }
    
    BOOL result = StartServiceA(service, 0, nullptr);
    CloseServiceHandle(service);
    return result != FALSE;
}

bool ServiceManager::stop_service(const std::string& service_name) {
    if (!service_manager_handle_) {
        return false;
    }
    
    SC_HANDLE service = OpenServiceA(service_manager_handle_, 
                                    service_name.c_str(), SERVICE_STOP);
    if (!service) {
        return false;
    }
    
    SERVICE_STATUS status;
    BOOL result = ControlService(service, SERVICE_CONTROL_STOP, &status);
    CloseServiceHandle(service);
    return result != FALSE;
}

bool ServiceManager::restart_service(const std::string& service_name) {
    stop_service(service_name);
    Sleep(1000);  // Wait for service to stop
    return start_service(service_name);
}

bool ServiceManager::pause_service(const std::string& service_name) {
    if (!service_manager_handle_) {
        return false;
    }
    
    SC_HANDLE service = OpenServiceA(service_manager_handle_, 
                                    service_name.c_str(), SERVICE_PAUSE_CONTINUE);
    if (!service) {
        return false;
    }
    
    SERVICE_STATUS status;
    BOOL result = ControlService(service, SERVICE_CONTROL_PAUSE, &status);
    CloseServiceHandle(service);
    return result != FALSE;
}

bool ServiceManager::resume_service(const std::string& service_name) {
    if (!service_manager_handle_) {
        return false;
    }
    
    SC_HANDLE service = OpenServiceA(service_manager_handle_, 
                                    service_name.c_str(), SERVICE_PAUSE_CONTINUE);
    if (!service) {
        return false;
    }
    
    SERVICE_STATUS status;
    BOOL result = ControlService(service, SERVICE_CONTROL_CONTINUE, &status);
    CloseServiceHandle(service);
    return result != FALSE;
}

std::string ServiceManager::get_service_status_string(uint32_t status) {
    switch (status) {
        case SERVICE_STOPPED: return "Stopped";
        case SERVICE_START_PENDING: return "Start Pending";
        case SERVICE_STOP_PENDING: return "Stop Pending";
        case SERVICE_RUNNING: return "Running";
        case SERVICE_CONTINUE_PENDING: return "Continue Pending";
        case SERVICE_PAUSE_PENDING: return "Pause Pending";
        case SERVICE_PAUSED: return "Paused";
        default: return "Unknown";
    }
}

std::string ServiceManager::get_service_start_type_string(uint32_t start_type) {
    switch (start_type) {
        case SERVICE_BOOT_START: return "Boot";
        case SERVICE_SYSTEM_START: return "System";
        case SERVICE_AUTO_START: return "Automatic";
        case SERVICE_DEMAND_START: return "Manual";
        case SERVICE_DISABLED: return "Disabled";
        default: return "Unknown";
    }
}

uint32_t ServiceManager::get_service_process_id(const std::string& service_name) {
    // Implementation would query service status
    return 0;
}

std::string ServiceInfo::to_json() const {
    std::ostringstream oss;
    oss << "{"
        << "\"service_name\":\"" << json_escape(service_name) << "\","
        << "\"display_name\":\"" << json_escape(display_name) << "\","
        << "\"description\":\"" << json_escape(description) << "\","
        << "\"status\":\"" << json_escape(status) << "\","
        << "\"startup_type\":\"" << json_escape(startup_type) << "\","
        << "\"binary_path\":\"" << json_escape(binary_path) << "\","
        << "\"process_id\":" << process_id
        << "}";
    return oss.str();
}

} // namespace smn