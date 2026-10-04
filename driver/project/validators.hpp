#pragma once
#include "core/status_codes.hpp"

namespace validators {

inline bool is_aligned(uint64_t addr, uint64_t alignment) {
    return (addr % alignment) == 0;
}

inline bool is_valid_kernel_address(void* addr) {
    if (!addr) return false;
    uint64_t addr_val = reinterpret_cast<uint64_t>(addr);
    return addr_val > 0xFFFF800000000000ULL;
}

inline bool is_valid_user_address(void* addr) {
    if (!addr) return false;
    uint64_t addr_val = reinterpret_cast<uint64_t>(addr);
    return addr_val < 0x0000800000000000ULL;
}

inline bool is_valid_pid(uint64_t pid) {
    return pid > 0 && pid <= 0xFFFFFFFFULL && (pid % 4) == 0;
}

inline bool is_valid_cr3(uint64_t cr3) {
    if (cr3 == 0) return false;
    return (cr3 & 0xFFF) == 0 || (cr3 & 0xF) != 0;
}

inline bool is_valid_size_for_copy(uint64_t size) {
    return size > 0 && size <= 0x100000 && (size % 4) == 0;
}

inline bool is_valid_module_name(const char* name, size_t max_len) {
    if (!name) return false;
    size_t len = strnlen(name, max_len);
    return len > 0 && len < max_len;
}

inline project_status validate_pointer(void* ptr, uint64_t size) {
    if (!ptr) return status_invalid_parameter;
    if (size == 0 || size > 0x100000000ULL) return status_mem_copy_size_invalid;
    return status_success;
}

inline project_status validate_cr3_pair(uint64_t src_cr3, uint64_t dst_cr3) {
    if (!is_valid_cr3(src_cr3)) return status_cr3_invalid_format;
    if (!is_valid_cr3(dst_cr3)) return status_cr3_invalid_format;
    return status_success;
}

inline project_status validate_copy_params(void* dst, void* src, uint64_t size, uint64_t dst_cr3, uint64_t src_cr3) {
    if (!is_valid_user_address(dst) || !is_valid_user_address(src)) {
        return status_invalid_parameter;
    }

    if (!is_valid_size_for_copy(size)) {
        return status_mem_copy_size_invalid;
    }

    if (!is_valid_cr3(src_cr3) || !is_valid_cr3(dst_cr3)) {
        return status_cr3_invalid_format;
    }

    if (!is_aligned(reinterpret_cast<uint64_t>(dst), 16) || 
        !is_aligned(reinterpret_cast<uint64_t>(src), 16)) {
        return status_mem_copy_alignment_failed;
    }

    return status_success;
}

inline project_status validate_process_name(const char* name) {
    if (!name) return status_invalid_parameter;
    
    size_t len = strnlen(name, MAX_PATH);
    if (len == 0 || len >= MAX_PATH) {
        return status_process_id_invalid;
    }

    for (size_t i = 0; i < len; i++) {
        char c = name[i];
        if (!isalnum(c) && c != '_' && c != '-' && c != '.') {
            return status_module_name_invalid;
        }
    }

    return status_success;
}

inline project_status validate_module_name(const char* name) {
    if (!name) return status_invalid_parameter;
    
    size_t len = strnlen(name, MAX_PATH);
    if (len == 0 || len >= MAX_PATH) {
        return status_module_name_invalid;
    }

    return status_success;
}

inline project_status validate_command_ptr(uint64_t ptr) {
    if (ptr < 0x1000) return status_data_ptr_invalid;
    if (!is_valid_user_address(reinterpret_cast<void*>(ptr))) {
        return status_invalid_parameter;
    }
    return status_success;
}

}
