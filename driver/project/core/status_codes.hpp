#pragma once
#include <cstdint>

enum project_status : uint32_t {
    status_success = 0x00000000,

    status_generic_failure = 0x00000001,
    status_invalid_parameter = 0x00000002,
    status_memory_allocation_failed = 0x00000003,
    status_win_address_translation_failed = 0x00000004,
    status_not_supported = 0x00000005,

    status_cr3_not_found = 0x00010000,
    status_cr3_invalid_format = 0x00010001,
    status_cr3_encrypted_unsupported = 0x00010002,
    status_cr3_access_denied = 0x00010003,
    status_cr3_mismatch = 0x00010004,

    status_mem_copy_timeout = 0x00020000,
    status_mem_copy_access_violation = 0x00020001,
    status_mem_copy_size_invalid = 0x00020002,
    status_mem_copy_alignment_failed = 0x00020003,
    status_mem_allocation_fragmentation = 0x00020004,

    status_process_not_found = 0x00030000,
    status_process_terminating = 0x00030001,
    status_process_access_denied = 0x00030002,
    status_process_id_invalid = 0x00030003,

    status_module_not_loaded = 0x00040000,
    status_module_name_invalid = 0x00040001,
    status_module_enumeration_failed = 0x00040002,
    status_module_base_not_found = 0x00040003,

    status_paging_invalid_idx = 0x00050000,
    status_paging_entry_not_present = 0x00050001,
    status_paging_entry_corrupted = 0x00050002,
    status_paging_wrong_granularity = 0x00050003,
    status_page_already_unmapped = 0x00050004,

    status_remapping_entry_found = 0x00060000,
    status_remapping_no_valid_entry = 0x00060001,
    status_remapping_list_full = 0x00060002,
    status_remapping_address_already_mapped = 0x00060003,
    status_remapping_failed = 0x00060004,

    status_non_aligned = 0x00060005,

    status_data_ptr_invalid = 0x00070000,
    status_data_ptr_corrupted = 0x00070001,
    status_gadget_not_found = 0x00070002,

    status_interrupt_init_failed = 0x00080000,
    status_idt_creation_failed = 0x00080001,
    status_nmi_handler_install_failed = 0x00080002,

    status_command_invalid_type = 0x00090000,
    status_command_checksum_mismatch = 0x00090001,
    status_command_timeout = 0x00090002,
    status_command_validation_failed = 0x00090003,

    status_cache_miss = 0x000A0000,
    status_cache_eviction = 0x000A0001,

    status_wrong_context = 0x000B0000,
    status_invalid_page_table = 0x000B0001,
    status_potential_unmapping_overflow = 0x000B0002,
};

inline const char* project_status_to_string(project_status status) {
    switch (status) {
        case status_success: return "SUCCESS";
        case status_generic_failure: return "GENERIC_FAILURE";
        case status_invalid_parameter: return "INVALID_PARAMETER";
        case status_memory_allocation_failed: return "MEMORY_ALLOCATION_FAILED";
        case status_win_address_translation_failed: return "ADDRESS_TRANSLATION_FAILED";
        case status_not_supported: return "NOT_SUPPORTED";
        case status_cr3_not_found: return "CR3_NOT_FOUND";
        case status_cr3_invalid_format: return "CR3_INVALID_FORMAT";
        case status_cr3_encrypted_unsupported: return "CR3_ENCRYPTED_UNSUPPORTED";
        case status_cr3_access_denied: return "CR3_ACCESS_DENIED";
        case status_mem_copy_timeout: return "MEM_COPY_TIMEOUT";
        case status_mem_copy_access_violation: return "MEM_COPY_ACCESS_VIOLATION";
        case status_mem_copy_size_invalid: return "MEM_COPY_SIZE_INVALID";
        case status_process_not_found: return "PROCESS_NOT_FOUND";
        case status_process_terminating: return "PROCESS_TERMINATING";
        case status_module_not_loaded: return "MODULE_NOT_LOADED";
        case status_module_name_invalid: return "MODULE_NAME_INVALID";
        default: return "UNKNOWN";
    }
}

inline bool is_valid_status(project_status status) {
    return status == status_success || status != 0;
}

inline bool is_error_status(project_status status) {
    return status != status_success;
}
