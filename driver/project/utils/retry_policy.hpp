#pragma once
#include "../core/status_codes.hpp"
#include "../project_includes.hpp"

namespace retry {

struct retry_config {
    uint32_t max_attempts;
    uint32_t initial_delay_ms;
    uint32_t max_delay_ms;
    bool use_exponential_backoff;
};

inline constexpr retry_config default_config() {
    return {3, 10, 100, true};
}

inline constexpr retry_config no_retry_config() {
    return {1, 0, 0, false};
}

template<typename TFunc>
inline project_status execute_with_retry(TFunc func, const retry_config& config) {
    project_status last_status = status_generic_failure;

    for (uint32_t attempt = 0; attempt < config.max_attempts; ++attempt) {
        last_status = func();

        if (last_status == status_success) {
            return status_success;
        }

        if (attempt < config.max_attempts - 1) {
            uint32_t delay_ms = config.initial_delay_ms;
            
            if (config.use_exponential_backoff) {
                for (uint32_t i = 0; i < attempt; ++i) {
                    delay_ms *= 2;
                }
                if (delay_ms > config.max_delay_ms) {
                    delay_ms = config.max_delay_ms;
                }
            }

            sleep(delay_ms);
        }
    }

    return last_status;
}

template<typename TFunc>
inline project_status copy_memory_with_retry(TFunc copy_func, const retry_config& config = default_config()) {
    return execute_with_retry(copy_func, config);
}

}
