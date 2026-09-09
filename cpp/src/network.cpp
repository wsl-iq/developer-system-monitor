/**
 * @file network.cpp
 * @brief Network monitoring and information retrieval.
 * @addtogroup Network
 * Copyright © 2026, inc. Mohammed Al-Baqer. All rights reserved.
 */

#include "network.hpp"
#include <windows.h>
#include <iphlpapi.h>
#include <ws2tcpip.h>
#include <sstream>
#include <iomanip>
#include <cstdio>
#include <cstring>

#pragma comment(lib, "iphlpapi.lib")
#pragma comment(lib, "ws2_32.lib")

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
                if (static_cast<unsigned char>(c) < 0x20U) {
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

NetworkMonitor::NetworkMonitor() : pdh_query_(nullptr), 
                                   network_received_counter_(nullptr),
                                   network_sent_counter_(nullptr) {
    initialize_counters();
}

NetworkMonitor::~NetworkMonitor() {
    cleanup_counters();
}

void NetworkMonitor::initialize_counters() {
    // Initialize network performance counters
}

void NetworkMonitor::cleanup_counters() {
    if (pdh_query_) {
        // Cleanup PDH query
        pdh_query_ = nullptr;
    }
}

std::vector<NetworkAdapterInfo> NetworkMonitor::get_network_adapters() {
    std::vector<NetworkAdapterInfo> adapters;
    
    // Get adapter information using GetAdaptersInfo
    ULONG buffer_size = 0;
    GetAdaptersInfo(nullptr, &buffer_size);
    
    if (buffer_size == 0) {
        return adapters;
    }
    
    std::vector<BYTE> buffer(buffer_size);
    PIP_ADAPTER_INFO adapter_info = (PIP_ADAPTER_INFO)buffer.data();
    
    if (GetAdaptersInfo(adapter_info, &buffer_size) == ERROR_SUCCESS) {
        while (adapter_info) {
            NetworkAdapterInfo info;
            info.name = adapter_info->AdapterName;
            info.description = adapter_info->Description;
            
            // Format MAC address
            std::ostringstream mac_oss;
            for (UINT i = 0; i < adapter_info->AddressLength; ++i) {
                if (i > 0) mac_oss << ":";
                mac_oss << std::hex << std::setw(2) << std::setfill('0')
                        << (int)adapter_info->Address[i];
            }
            info.mac_address = mac_oss.str();
            
            // Get IP addresses
            PIP_ADDR_STRING addr = &adapter_info->IpAddressList;
            while (addr) {
                if (strlen(addr->IpAddress.String) > 0) {
                    info.ip_addresses.push_back(addr->IpAddress.String);
                }
                addr = addr->Next;
            }
            
            // Get adapter type
            switch (adapter_info->Type) {
                case MIB_IF_TYPE_ETHERNET:
                    info.adapter_type = "Ethernet";
                    break;
                case MIB_IF_TYPE_OTHER:
                    info.adapter_type = "Other";
                    break;
                default:
                    info.adapter_type = "Unknown";
            }
            
            info.status = "Up";  // Simplified - should check actual status
            
            adapters.push_back(info);
            adapter_info = adapter_info->Next;
        }
    }
    
    return adapters;
}

std::vector<NetworkAdapterInfo> NetworkMonitor::get_network_usage() {
    auto adapters = get_network_adapters();
    
    // Add traffic statistics
    // This will be implemented with PDH counters in the full version
    for (auto& adapter : adapters) {
        adapter.bytes_received = 0;
        adapter.bytes_sent = 0;
        adapter.bytes_received_per_sec = 0;
        adapter.bytes_sent_per_sec = 0;
        adapter.packets_received = 0;
        adapter.packets_sent = 0;
    }
    
    return adapters;
}

std::string NetworkAdapterInfo::to_json() const {
    std::ostringstream oss;
    oss << "{"
        << "\"name\":\"" << name << "\","
        << "\"description\":\"" << description << "\","
        << "\"mac_address\":\"" << mac_address << "\","
        << "\"adapter_type\":\"" << adapter_type << "\","
        << "\"status\":\"" << status << "\","
        << "\"speed_bps\":" << speed_bps << ","
        << "\"bytes_received\":" << bytes_received << ","
        << "\"bytes_sent\":" << bytes_sent << ","
        << "\"bytes_received_per_sec\":" << bytes_received_per_sec << ","
        << "\"bytes_sent_per_sec\":" << bytes_sent_per_sec << ","
        << "\"packets_received\":" << packets_received << ","
        << "\"packets_sent\":" << packets_sent;
    
    // Add IP addresses
    oss << ",\"ip_addresses\":[";
    for (size_t i = 0; i < ip_addresses.size(); ++i) {
        if (i > 0) oss << ",";
        oss << "\"" << ip_addresses[i] << "\"";
    }
    oss << "]";
    
    oss << "}";
    return oss.str();
}

} // namespace smn