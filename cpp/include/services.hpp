/**
 * @file services.hpp
 * @brief services monitoring and information retrieval.
 * @addtogroup services
 * Copyright © 2026, inc. Mohammed Al-Baqer. All rights reserved.
 */

#pragma once

#include <string>
#include <vector>
#include <cstdint>
#include <windows.h>
#include <winsvc.h>

namespace smn {

struct ServiceInfo {
    std::string service_name;
    std::string display_name;
    std::string description;
    std::string status;
    std::string startup_type;
    std::string binary_path;
    uint32_t process_id;
    
    std::string to_json() const;
};

class ServiceManager {
public:
    ServiceManager();
    ~ServiceManager();
    
    std::vector<ServiceInfo> get_service_list();
    ServiceInfo get_service_info(const std::string& service_name);
    
    bool start_service(const std::string& service_name);
    bool stop_service(const std::string& service_name);
    bool restart_service(const std::string& service_name);
    bool pause_service(const std::string& service_name);
    bool resume_service(const std::string& service_name);
    
private:
    SC_HANDLE service_manager_handle_;
    
    void initialize();
    void cleanup();
    
    // Helper functions
    std::string get_service_status_string(uint32_t status);
    std::string get_service_start_type_string(uint32_t start_type);
    uint32_t get_service_process_id(const std::string& service_name);
};

} // namespace smn