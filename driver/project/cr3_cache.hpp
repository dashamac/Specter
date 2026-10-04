#pragma once
#include "core/status_codes.hpp"
#include <ntdef.h>
#include <new>

namespace cr3_cache {

constexpr size_t CACHE_SIZE = 256;
constexpr uint64_t CACHE_TTL_US = 5000000;

struct cache_entry {
    uint64_t pid;
    uint64_t cr3;
    uint64_t timestamp;

    bool is_valid() const {
        uint64_t now = KeQueryInterruptTime();
        return (now - timestamp) < CACHE_TTL_US;
    }

    bool matches_pid(uint64_t target_pid) const {
        return pid == target_pid && is_valid();
    }
};

class CR3Cache {
private:
    cache_entry entries[CACHE_SIZE];
    volatile size_t head;
    KSPIN_LOCK lock;

public:
    CR3Cache() : head(0) {
        KeInitializeSpinLock(&lock);
        memset(entries, 0, sizeof(entries));
    }

    ~CR3Cache() = default;

    bool try_get(uint64_t pid, uint64_t& out_cr3) {
        KIRQL old_irql;
        KeAcquireSpinLock(&lock, &old_irql);

        for (size_t i = 0; i < CACHE_SIZE; ++i) {
            if (entries[i].matches_pid(pid)) {
                out_cr3 = entries[i].cr3;
                KeReleaseSpinLock(&lock, old_irql);
                return true;
            }
        }

        KeReleaseSpinLock(&lock, old_irql);
        return false;
    }

    void insert(uint64_t pid, uint64_t cr3) {
        KIRQL old_irql;
        KeAcquireSpinLock(&lock, &old_irql);

        size_t idx = head;
        entries[idx].pid = pid;
        entries[idx].cr3 = cr3;
        entries[idx].timestamp = KeQueryInterruptTime();
        head = (head + 1) % CACHE_SIZE;

        KeReleaseSpinLock(&lock, old_irql);
    }

    void clear() {
        KIRQL old_irql;
        KeAcquireSpinLock(&lock, &old_irql);
        memset(entries, 0, sizeof(entries));
        head = 0;
        KeReleaseSpinLock(&lock, old_irql);
    }
};

inline CR3Cache* g_cr3_cache = nullptr;

inline project_status initialize_cache() {
    if (g_cr3_cache) {
        return status_success;
    }

    g_cr3_cache = (CR3Cache*)ExAllocatePoolWithTag(
        NonPagedPoolNx, sizeof(CR3Cache), 'CR3C');

    if (!g_cr3_cache) {
        return status_memory_allocation_failed;
    }

    new (g_cr3_cache) CR3Cache();
    return status_success;
}

inline void cleanup_cache() {
    if (g_cr3_cache) {
        g_cr3_cache->~CR3Cache();
        ExFreePoolWithTag(g_cr3_cache, 'CR3C');
        g_cr3_cache = nullptr;
    }
}

}
