/**
 * @file process.cpp
 * @brief Process monitoring and information retrieval.
 * @addtogroup Process
 * Copyright © 2026, inc. Mohammed Al-Baqer. All rights reserved.
 */

#include "process.hpp"
#include <windows.h>
#include <tlhelp32.h>
#include <psapi.h>
#include <sstream>
#include <iomanip>
#include <unordered_map>
#include <algorithm>
#include <chrono>
#include <cstdio>

#pragma comment(lib, "psapi.lib")
#pragma comment(lib, "advapi32.lib")

namespace smn {

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
            case '\b': result += "\\b"; break;
            case '\f': result += "\\f"; break;
            default:
                if (static_cast<unsigned char>(c) < 0x20) {
                    char buf[8];
                    std::snprintf(buf, sizeof(buf), "\\u%04x", static_cast<unsigned char>(c));
                    result += buf;
                } else {
                    result += c;
                }
                break;
        }
    }

    return result;
}

ProcessManager::ProcessManager() : pdh_query_(nullptr) {
    initialize_counters();
    last_sample_time_ = std::chrono::steady_clock::now();
}

ProcessManager::~ProcessManager() {
    cleanup_counters();
}

void ProcessManager::initialize_counters() {
    // Initialize process monitoring
}

void ProcessManager::cleanup_counters() {
    if (pdh_query_) {
        // Cleanup PDH query
        pdh_query_ = nullptr;
    }
}

std::vector<ProcessInfo> ProcessManager::get_process_list() {
    std::vector<ProcessInfo> processes;
    
    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snapshot == INVALID_HANDLE_VALUE) {
        return processes;
    }
    
    PROCESSENTRY32W process_entry;
    process_entry.dwSize = sizeof(process_entry);
    
    if (Process32FirstW(snapshot, &process_entry)) {
        do {
            ProcessInfo info;
            info.pid = process_entry.th32ProcessID;
            info.parent_pid = process_entry.th32ParentProcessID;
            
            // Convert wide string to narrow string
            char name[MAX_PATH];
            WideCharToMultiByte(CP_UTF8, 0, process_entry.szExeFile, -1, 
                               name, MAX_PATH, nullptr, nullptr);
            info.name = name;
            
            info.memory_usage_bytes = get_process_memory(info.pid);
            info.executable_path = get_process_path(info.pid);
            info.cpu_usage_percent = get_process_cpu_usage(info.pid);
            info.user_name = get_process_user(info.pid);
            info.thread_count = process_entry.cntThreads;
            info.status = "Running";
            info.priority_class = 0;
            info.peak_memory_usage_bytes = 0;
            info.disk_read_bytes = 0;
            info.disk_write_bytes = 0;
            
            // Get process creation time
            HANDLE process_handle = OpenProcess(PROCESS_QUERY_INFORMATION, 
                                               FALSE, info.pid);
            if (process_handle) {
                FILETIME create_time, exit_time, kernel_time, user_time;
                if (GetProcessTimes(process_handle, &create_time, &exit_time,
                                   &kernel_time, &user_time)) {
                    ULARGE_INTEGER uli;
                    uli.LowPart = create_time.dwLowDateTime;
                    uli.HighPart = create_time.dwHighDateTime;
                    info.create_time = uli.QuadPart;
                }
                
                // Get priority class
                info.priority_class = GetPriorityClass(process_handle);
                
                CloseHandle(process_handle);
            }
            
            processes.push_back(info);
        } while (Process32NextW(snapshot, &process_entry));
    }
    
    CloseHandle(snapshot);
    return processes;
}

ProcessInfo ProcessManager::get_process_info(uint32_t pid) {
    ProcessInfo info;
    info.pid = pid;
    
    HANDLE process_handle = OpenProcess(PROCESS_QUERY_INFORMATION | 
                                        PROCESS_VM_READ, FALSE, pid);
    if (process_handle) {
        char path[MAX_PATH];
        DWORD size = MAX_PATH;
        if (QueryFullProcessImageNameA(process_handle, 0, path, &size)) {
            info.executable_path = path;
        }
        
        // Get process name from path
        std::string path_str = info.executable_path;
        size_t pos = path_str.find_last_of('\\');
        if (pos != std::string::npos) {
            info.name = path_str.substr(pos + 1);
        }
        
        info.memory_usage_bytes = get_process_memory(pid);
        info.cpu_usage_percent = get_process_cpu_usage(pid);
        info.user_name = get_process_user(pid);
        info.priority_class = GetPriorityClass(process_handle);
        info.status = "Running";
        
        // Get creation time
        FILETIME create_time, exit_time, kernel_time, user_time;
        if (GetProcessTimes(process_handle, &create_time, &exit_time,
                           &kernel_time, &user_time)) {
            ULARGE_INTEGER uli;
            uli.LowPart = create_time.dwLowDateTime;
            uli.HighPart = create_time.dwHighDateTime;
            info.create_time = uli.QuadPart;
        }
        
        CloseHandle(process_handle);
    }
    
    return info;
}

bool ProcessManager::terminate_process(uint32_t pid) {
    HANDLE process_handle = OpenProcess(PROCESS_TERMINATE, FALSE, pid);
    if (!process_handle) {
        return false;
    }
    
    BOOL result = TerminateProcess(process_handle, 1);
    CloseHandle(process_handle);
    return result != FALSE;
}

bool ProcessManager::suspend_process(uint32_t pid) {
    // This requires getting all threads of the process and suspending them
    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
    if (snapshot == INVALID_HANDLE_VALUE) {
        return false;
    }
    
    THREADENTRY32 thread_entry;
    thread_entry.dwSize = sizeof(thread_entry);
    
    if (Thread32First(snapshot, &thread_entry)) {
        do {
            if (thread_entry.th32OwnerProcessID == pid) {
                HANDLE thread_handle = OpenThread(THREAD_SUSPEND_RESUME, 
                                                 FALSE, thread_entry.th32ThreadID);
                if (thread_handle) {
                    SuspendThread(thread_handle);
                    CloseHandle(thread_handle);
                }
            }
        } while (Thread32Next(snapshot, &thread_entry));
    }
    
    CloseHandle(snapshot);
    return true;
}

bool ProcessManager::resume_process(uint32_t pid) {
    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
    if (snapshot == INVALID_HANDLE_VALUE) {
        return false;
    }
    
    THREADENTRY32 thread_entry;
    thread_entry.dwSize = sizeof(thread_entry);
    
    if (Thread32First(snapshot, &thread_entry)) {
        do {
            if (thread_entry.th32OwnerProcessID == pid) {
                HANDLE thread_handle = OpenThread(THREAD_SUSPEND_RESUME, 
                                                 FALSE, thread_entry.th32ThreadID);
                if (thread_handle) {
                    ResumeThread(thread_handle);
                    CloseHandle(thread_handle);
                }
            }
        } while (Thread32Next(snapshot, &thread_entry));
    }
    
    CloseHandle(snapshot);
    return true;
}

bool ProcessManager::set_process_priority(uint32_t pid, uint32_t priority_class) {
    HANDLE process_handle = OpenProcess(PROCESS_SET_INFORMATION, FALSE, pid);
    if (!process_handle) {
        return false;
    }
    
    BOOL result = SetPriorityClass(process_handle, priority_class);
    CloseHandle(process_handle);
    return result != FALSE;
}

double ProcessManager::get_process_cpu_usage(uint32_t pid) {
    HANDLE process_handle = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, pid);
    if (!process_handle) {
        return 0.0;
    }
    
    FILETIME create_time, exit_time, kernel_time, user_time;
    if (GetProcessTimes(process_handle, &create_time, &exit_time,
                        &kernel_time, &user_time)) {
        ULARGE_INTEGER kernel;
        kernel.LowPart = kernel_time.dwLowDateTime;
        kernel.HighPart = kernel_time.dwHighDateTime;
        
        ULARGE_INTEGER user;
        user.LowPart = user_time.dwLowDateTime;
        user.HighPart = user_time.dwHighDateTime;
        
        uint64_t total_time = kernel.QuadPart + user.QuadPart;
        
        // Store sample for next calculation
        auto now = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
            now - last_sample_time_).count() / 1000.0;
        
        if (elapsed > 0 && process_cpu_samples_.count(pid) > 0) {
            uint64_t prev_time = process_cpu_samples_[pid].cpu_time;
            double cpu_usage = (total_time - prev_time) / (elapsed * 10000.0);
            process_cpu_samples_[pid] = {total_time, now};
            CloseHandle(process_handle);
            return cpu_usage;
        }
        
        process_cpu_samples_[pid] = {total_time, now};
    }
    
    CloseHandle(process_handle);
    return 0.0;
}

uint64_t ProcessManager::get_process_memory(uint32_t pid) {
    HANDLE process_handle = OpenProcess(PROCESS_QUERY_INFORMATION | 
                                        PROCESS_VM_READ, FALSE, pid);
    if (!process_handle) {
        return 0;
    }
    
    PROCESS_MEMORY_COUNTERS_EX pmc;
    pmc.cb = sizeof(pmc);
    
    uint64_t memory = 0;
    if (GetProcessMemoryInfo(process_handle, 
                            (PROCESS_MEMORY_COUNTERS*)&pmc, sizeof(pmc))) {
        memory = pmc.WorkingSetSize;
    }
    
    CloseHandle(process_handle);
    return memory;
}

std::string ProcessManager::get_process_path(uint32_t pid) {
    std::string path;
    HANDLE process_handle = OpenProcess(PROCESS_QUERY_INFORMATION | 
                                        PROCESS_VM_READ, FALSE, pid);
    if (process_handle) {
        char buffer[MAX_PATH];
        DWORD size = MAX_PATH;
        if (QueryFullProcessImageNameA(process_handle, 0, buffer, &size)) {
            path = buffer;
        }
        CloseHandle(process_handle);
    }
    return path;
}

std::string ProcessManager::get_process_user(uint32_t pid) {
    std::string user_name;
    HANDLE process_handle = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, pid);
    if (process_handle) {
        HANDLE token;
        if (OpenProcessToken(process_handle, TOKEN_QUERY, &token)) {
            DWORD size = 0;
            GetTokenInformation(token, TokenUser, nullptr, 0, &size);
            if (size > 0) {
                std::vector<BYTE> buffer(size);
                if (GetTokenInformation(token, TokenUser, buffer.data(), 
                                       size, &size)) {
                    TOKEN_USER* token_user = (TOKEN_USER*)buffer.data();
                    char name[256], domain[256];
                    DWORD name_size = sizeof(name), domain_size = sizeof(domain);
                    SID_NAME_USE sid_type;
                    if (LookupAccountSidA(nullptr, token_user->User.Sid,
                                         name, &name_size, domain, &domain_size,
                                         &sid_type)) {
                        user_name = std::string(domain) + "\\" + name;
                    }
                }
            }
            CloseHandle(token);
        }
        CloseHandle(process_handle);
    }
    return user_name;
}

std::vector<uint32_t> ProcessManager::enumerate_process_ids() {
    std::vector<uint32_t> pids;
    DWORD process_ids[4096];
    DWORD bytes_returned;
    
    if (EnumProcesses(process_ids, sizeof(process_ids), &bytes_returned)) {
        DWORD count = bytes_returned / sizeof(DWORD);
        for (DWORD i = 0; i < count; ++i) {
            pids.push_back(process_ids[i]);
        }
    }
    
    return pids;
}

std::string ProcessInfo::to_json() const {
    std::ostringstream oss;
    oss << "{"
        << "\"pid\":" << pid << ","
        << "\"parent_pid\":" << parent_pid << ","
        << "\"name\":\"" << json_escape(name) << "\","
        << "\"executable_path\":\"" << json_escape(executable_path) << "\","
        << "\"user_name\":\"" << json_escape(user_name) << "\","
        << "\"status\":\"" << json_escape(status) << "\","
        << "\"priority_class\":" << priority_class << ","
        << "\"memory_usage_bytes\":" << memory_usage_bytes << ","
        << "\"peak_memory_usage_bytes\":" << peak_memory_usage_bytes << ","
        << "\"cpu_usage_percent\":" << std::fixed << std::setprecision(2) 
        << cpu_usage_percent << ","
        << "\"create_time\":" << create_time << ","
        << "\"thread_count\":" << thread_count << ","
        << "\"disk_read_bytes\":" << disk_read_bytes << ","
        << "\"disk_write_bytes\":" << disk_write_bytes
        << "}";
    return oss.str();
}

} // namespace smn