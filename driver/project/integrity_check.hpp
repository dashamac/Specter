#pragma once
#include "core/status_codes.hpp"

namespace integrity {

inline uint32_t crc32_compute(const uint8_t* data, size_t length) {
    uint32_t crc = 0xFFFFFFFFU;
    
    static const uint32_t polynomial = 0xEDB88320U;

    for (size_t i = 0; i < length; ++i) {
        crc ^= data[i];
        for (int j = 0; j < 8; ++j) {
            crc = (crc >> 1) ^ (polynomial & -(int32_t)(crc & 1));
        }
    }

    return crc ^ 0xFFFFFFFFU;
}

inline uint32_t compute_command_checksum(const void* cmd_data, size_t size) {
    const uint8_t* data = (const uint8_t*)cmd_data;
    return crc32_compute(data, size);
}

inline project_status validate_command_checksum(const void* cmd_data, size_t data_size, 
                                                uint32_t expected_checksum) {
    uint32_t computed = compute_command_checksum(cmd_data, data_size);
    
    if (computed != expected_checksum) {
        return status_command_checksum_mismatch;
    }

    return status_success;
}

inline project_status verify_pointer_validity(void* ptr, size_t size) {
    if (!ptr || size == 0) {
        return status_invalid_parameter;
    }

    __try {
        ProbeForRead(ptr, size, sizeof(void*));
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
        return status_mem_copy_access_violation;
    }

    return status_success;
}

}
