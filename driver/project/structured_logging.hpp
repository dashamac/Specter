#pragma once
#include "core/status_codes.hpp"
#include <ntdef.h>
#include <new>

namespace structured_logging {

enum severity_level : uint16_t {
    severity_debug = 0,
    severity_info = 1,
    severity_warning = 2,
    severity_error = 3,
};

struct log_entry {
    uint64_t timestamp;
    uint32_t command_type;
    uint16_t severity;
    uint16_t cpu_id;
    uint32_t duration_us;
    uint32_t result_code;
    char message[256];
};

struct telemetry_counters {
    uint64_t cmd_get_pid_count;
    uint64_t cmd_get_pid_errors;
    uint64_t cmd_get_pid_total_us;

    uint64_t cmd_get_cr3_count;
    uint64_t cmd_get_cr3_errors;
    uint64_t cmd_get_cr3_total_us;

    uint64_t cmd_copy_memory_count;
    uint64_t cmd_copy_memory_bytes;
    uint64_t cmd_copy_memory_errors;
    uint64_t cmd_copy_memory_total_us;

    uint64_t cmd_get_module_base_count;
    uint64_t cmd_get_module_base_errors;

    uint64_t cmd_get_module_size_count;
    uint64_t cmd_get_module_size_errors;

    uint64_t total_commands;
    uint64_t total_errors;
};

class StructuredLogger {
private:
    static constexpr size_t LOG_BUFFER_SIZE = 512;
    log_entry log_buffer[LOG_BUFFER_SIZE];
    volatile size_t current_idx;
    telemetry_counters telemetry;
    KSPIN_LOCK lock;

public:
    StructuredLogger() : current_idx(0) {
        KeInitializeSpinLock(&lock);
        memset(log_buffer, 0, sizeof(log_buffer));
        memset(&telemetry, 0, sizeof(telemetry));
    }

    void log_command(uint32_t cmd_type, severity_level severity, 
                     uint32_t duration_us, project_status status, 
                     const char* message) {
        KIRQL old_irql;
        KeAcquireSpinLock(&lock, &old_irql);

        size_t idx = current_idx;
        log_entry& entry = log_buffer[idx];

        entry.timestamp = KeQueryPerformanceCounter(nullptr).QuadPart;
        entry.command_type = cmd_type;
        entry.severity = severity;
        entry.cpu_id = KeGetCurrentProcessorNumber();
        entry.duration_us = duration_us;
        entry.result_code = status;

        if (message) {
            strncpy_s(entry.message, sizeof(entry.message), message, 
                     sizeof(entry.message) - 1);
        }

        current_idx = (current_idx + 1) % LOG_BUFFER_SIZE;
        KeReleaseSpinLock(&lock, old_irql);
    }

    void update_telemetry_get_pid(uint32_t duration_us, bool failed) {
        KIRQL old_irql;
        KeAcquireSpinLock(&lock, &old_irql);

        telemetry.cmd_get_pid_count++;
        telemetry.cmd_get_pid_total_us += duration_us;
        telemetry.total_commands++;

        if (failed) {
            telemetry.cmd_get_pid_errors++;
            telemetry.total_errors++;
        }

        KeReleaseSpinLock(&lock, old_irql);
    }

    void update_telemetry_get_cr3(uint32_t duration_us, bool failed) {
        KIRQL old_irql;
        KeAcquireSpinLock(&lock, &old_irql);

        telemetry.cmd_get_cr3_count++;
        telemetry.cmd_get_cr3_total_us += duration_us;
        telemetry.total_commands++;

        if (failed) {
            telemetry.cmd_get_cr3_errors++;
            telemetry.total_errors++;
        }

        KeReleaseSpinLock(&lock, old_irql);
    }

    void update_telemetry_copy_memory(uint32_t duration_us, uint64_t bytes, bool failed) {
        KIRQL old_irql;
        KeAcquireSpinLock(&lock, &old_irql);

        telemetry.cmd_copy_memory_count++;
        telemetry.cmd_copy_memory_bytes += bytes;
        telemetry.cmd_copy_memory_total_us += duration_us;
        telemetry.total_commands++;

        if (failed) {
            telemetry.cmd_copy_memory_errors++;
            telemetry.total_errors++;
        }

        KeReleaseSpinLock(&lock, old_irql);
    }

    void update_telemetry_get_module_base(bool failed) {
        KIRQL old_irql;
        KeAcquireSpinLock(&lock, &old_irql);

        telemetry.cmd_get_module_base_count++;
        telemetry.total_commands++;

        if (failed) {
            telemetry.cmd_get_module_base_errors++;
            telemetry.total_errors++;
        }

        KeReleaseSpinLock(&lock, old_irql);
    }

    void update_telemetry_get_module_size(bool failed) {
        KIRQL old_irql;
        KeAcquireSpinLock(&lock, &old_irql);

        telemetry.cmd_get_module_size_count++;
        telemetry.total_commands++;

        if (failed) {
            telemetry.cmd_get_module_size_errors++;
            telemetry.total_errors++;
        }

        KeReleaseSpinLock(&lock, old_irql);
    }

    const telemetry_counters* get_telemetry() const {
        return &telemetry;
    }

    const log_entry* get_logs() const {
        return log_buffer;
    }

    size_t get_log_count() const {
        return LOG_BUFFER_SIZE;
    }

    void clear() {
        KIRQL old_irql;
        KeAcquireSpinLock(&lock, &old_irql);
        memset(log_buffer, 0, sizeof(log_buffer));
        memset(&telemetry, 0, sizeof(telemetry));
        current_idx = 0;
        KeReleaseSpinLock(&lock, old_irql);
    }
};

inline StructuredLogger* g_logger = nullptr;

inline project_status initialize_logger() {
    if (g_logger) {
        return status_success;
    }

    g_logger = (StructuredLogger*)ExAllocatePoolWithTag(
        NonPagedPoolNx, sizeof(StructuredLogger), 'SLOG');

    if (!g_logger) {
        return status_memory_allocation_failed;
    }

    new (g_logger) StructuredLogger();
    return status_success;
}

inline void cleanup_logger() {
    if (g_logger) {
        g_logger->~StructuredLogger();
        ExFreePoolWithTag(g_logger, 'SLOG');
        g_logger = nullptr;
    }
}

}
