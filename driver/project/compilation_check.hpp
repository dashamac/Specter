#pragma once

namespace compilation_check {

template<typename T>
class type_check {
public:
    static_assert(sizeof(T) > 0, "Type size must be positive");
};

#define COMPILE_TIME_ASSERT(condition, message) \
    static_assert(condition, message)

COMPILE_TIME_ASSERT(sizeof(project_status) == 4, "project_status must be 4 bytes");

COMPILE_TIME_ASSERT(
    static_cast<int>(status_success) == 0x00000000,
    "status_success must be 0"
);

COMPILE_TIME_ASSERT(
    static_cast<int>(status_cr3_invalid_format) == 0x00010001,
    "status_cr3_invalid_format value changed"
);

COMPILE_TIME_ASSERT(
    static_cast<int>(status_mem_copy_size_invalid) == 0x00020002,
    "status_mem_copy_size_invalid value changed"
);

inline constexpr bool validate_cache_size() {
    return cr3_cache::CACHE_SIZE == 256;
}

inline constexpr bool validate_log_size() {
    return structured_logging::StructuredLogger::LOG_BUFFER_SIZE == 512;
}

}
