/**
 * @file network.hpp
 * @brief Network monitoring and information retrieval.
 * @addtogroup Network
 * Copyright © 2026, inc. Mohammed Al-Baqer. All rights reserved.
 */

#pragma once

#include <string>
#include <vector>
#include <cstdint>

namespace smn {

struct NetworkAdapterInfo {
    std::string name;
    std::string description;
    std::string mac_address;
    std::vector<std::string> ip_addresses;
    std::string adapter_type;
    std::string status;
    uint64_t speed_bps;
    
    // Traffic statistics
    uint64_t bytes_received;
    uint64_t bytes_sent;
    uint64_t bytes_received_per_sec;
    uint64_t bytes_sent_per_sec;
    uint64_t packets_received;
    uint64_t packets_sent;
    
    std::string to_json() const;
};

class NetworkMonitor {
public:
    NetworkMonitor();
    ~NetworkMonitor();
    
    std::vector<NetworkAdapterInfo> get_network_adapters();
    std::vector<NetworkAdapterInfo> get_network_usage();
    
private:
    void* pdh_query_;
    void* network_received_counter_;
    void* network_sent_counter_;
    
    void initialize_counters();
    void cleanup_counters();
    
    std::string get_adapter_mac(const std::string& adapter_name);
    std::vector<std::string> get_adapter_ips(const std::string& adapter_name);
};

} // namespace smn