#pragma once
#include "../core/status_codes.hpp"
#include "../utils/validators.hpp"
#include "../cache/cr3_cache.hpp"

namespace unit_tests {

class TestRunner {
private:
    uint32_t tests_passed;
    uint32_t tests_failed;

    void log_result(const char* test_name, bool passed) {
        if (passed) {
            DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, 
                      "[+] PASS: %s\n", test_name);
            tests_passed++;
        } else {
            DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, 
                      "[-] FAIL: %s\n", test_name);
            tests_failed++;
        }
    }

public:
    TestRunner() : tests_passed(0), tests_failed(0) {}

    void test_validators_valid_pid() {
        bool result = validators::is_valid_pid(0x4);
        log_result("validators::is_valid_pid(valid)", result);
    }

    void test_validators_invalid_pid_zero() {
        bool result = !validators::is_valid_pid(0);
        log_result("validators::is_valid_pid(zero)", result);
    }

    void test_validators_invalid_pid_odd() {
        bool result = !validators::is_valid_pid(0x3);
        log_result("validators::is_valid_pid(odd)", result);
    }

    void test_validators_valid_cr3() {
        bool result = validators::is_valid_cr3(0x1000);
        log_result("validators::is_valid_cr3(valid)", result);
    }

    void test_validators_invalid_cr3_zero() {
        bool result = !validators::is_valid_cr3(0);
        log_result("validators::is_valid_cr3(zero)", result);
    }

    void test_validators_valid_size_for_copy() {
        bool result = validators::is_valid_size_for_copy(0x1000);
        log_result("validators::is_valid_size_for_copy(valid)", result);
    }

    void test_validators_size_too_large() {
        bool result = !validators::is_valid_size_for_copy(0x200000);
        log_result("validators::is_valid_size_for_copy(too_large)", result);
    }

    void test_validators_aligned() {
        bool result = validators::is_aligned(0x1000, 0x1000);
        log_result("validators::is_aligned(true)", result);
    }

    void test_validators_not_aligned() {
        bool result = !validators::is_aligned(0x1001, 0x1000);
        log_result("validators::is_aligned(false)", result);
    }

    void test_validators_kernel_address() {
        bool result = validators::is_valid_kernel_address((void*)0xFFFF800000000000ULL);
        log_result("validators::is_valid_kernel_address(true)", result);
    }

    void test_validators_user_address() {
        bool result = validators::is_valid_user_address((void*)0x0000400000000000ULL);
        log_result("validators::is_valid_user_address(true)", result);
    }

    void test_cr3_cache_insert_and_retrieve() {
        if (!cr3_cache::g_cr3_cache) {
            log_result("cr3_cache_insert_and_retrieve", false);
            return;
        }

        cr3_cache::g_cr3_cache->insert(1234, 0x5000);
        uint64_t retrieved_cr3 = 0;
        bool found = cr3_cache::g_cr3_cache->try_get(1234, retrieved_cr3);

        bool result = found && retrieved_cr3 == 0x5000;
        log_result("cr3_cache_insert_and_retrieve", result);
    }

    void test_cr3_cache_miss() {
        if (!cr3_cache::g_cr3_cache) {
            log_result("cr3_cache_miss", false);
            return;
        }

        uint64_t retrieved_cr3 = 0;
        bool found = cr3_cache::g_cr3_cache->try_get(9999, retrieved_cr3);

        bool result = !found;
        log_result("cr3_cache_miss", result);
    }

    void run_all_tests() {
        DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, 
                  "===== Starting Unit Tests =====\n");

        test_validators_valid_pid();
        test_validators_invalid_pid_zero();
        test_validators_invalid_pid_odd();
        test_validators_valid_cr3();
        test_validators_invalid_cr3_zero();
        test_validators_valid_size_for_copy();
        test_validators_size_too_large();
        test_validators_aligned();
        test_validators_not_aligned();
        test_validators_kernel_address();
        test_validators_user_address();
        test_cr3_cache_insert_and_retrieve();
        test_cr3_cache_miss();

        DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, 
                  "===== Tests Complete: %u passed, %u failed =====\n", 
                  tests_passed, tests_failed);
    }

    uint32_t get_passed() const { return tests_passed; }
    uint32_t get_failed() const { return tests_failed; }
};

}
