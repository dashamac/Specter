#include "command_handler.hpp"
#include "../validators.hpp"
#include "../retry_policy.hpp"
#include "../cr3_cache.hpp"
#include "../structured_logging.hpp"
#include "../integrity_check.hpp"
#include "../project_api.hpp"
#include "../project_utility.hpp"
#include "../cr3 decryption/cr3_decryption.hpp"
#include <ntdef.h>

extern "C" NTKERNELAPI NTSTATUS PsLookupProcessByProcessId(_In_ HANDLE ProcessId, _Outptr_ PEPROCESS* Process);
extern "C" NTKERNELAPI PVOID PsGetProcessSectionBaseAddress(_In_ PEPROCESS Process);

namespace command_handlers {

project_status GetPidByNameHandler::execute(const command_t& cmd, uint64_t user_cr3) {
    LARGE_INTEGER start_time = KeQueryPerformanceCounter(nullptr);

    auto status = validators::validate_command_ptr(reinterpret_cast<uint64_t>(cmd.sub_command_ptr));
    if (status != status_success) {
        return status;
    }

    get_pid_by_name_t sub_cmd = {0};
    status = physmem::runtime::copy_memory_to_constructed_cr3(&sub_cmd, cmd.sub_command_ptr, 
                                                              sizeof(sub_cmd), user_cr3);
    if (status != status_success) {
        return status;
    }

    status = validators::validate_process_name(sub_cmd.name);
    if (status != status_success) {
        return status;
    }

    sub_cmd.pid = cr3_decryption::eproc::get_pid(sub_cmd.name);
    
    if (!sub_cmd.pid) {
        PEPROCESS target_eproc = nullptr;
        if (utility::get_eprocess(sub_cmd.name, target_eproc) == status_success && target_eproc) {
            sub_cmd.pid = *(uint64_t*)((uint8_t*)target_eproc + 0x440);
        }
    }

    if (!sub_cmd.pid) {
        status = status_process_not_found;
    }

    if (status == status_success) {
        status = physmem::runtime::copy_memory_from_constructed_cr3(
            cmd.sub_command_ptr, &sub_cmd, sizeof(sub_cmd), user_cr3);
    }

    LARGE_INTEGER end_time = KeQueryPerformanceCounter(nullptr);
    uint32_t duration_us = (uint32_t)((end_time.QuadPart - start_time.QuadPart) / 10);

    if (structured_logging::g_logger) {
        structured_logging::g_logger->update_telemetry_get_pid(duration_us, status != status_success);
    }

    return status;
}

project_status GetCR3Handler::execute(const command_t& cmd, uint64_t user_cr3) {
    LARGE_INTEGER start_time = KeQueryPerformanceCounter(nullptr);

    auto status = validators::validate_command_ptr(reinterpret_cast<uint64_t>(cmd.sub_command_ptr));
    if (status != status_success) {
        return status;
    }

    get_cr3_t sub_cmd = {0};
    status = physmem::runtime::copy_memory_to_constructed_cr3(&sub_cmd, cmd.sub_command_ptr, 
                                                              sizeof(sub_cmd), user_cr3);
    if (status != status_success) {
        return status;
    }

    if (!validators::is_valid_pid(sub_cmd.pid)) {
        return status_process_id_invalid;
    }

    if (cr3_cache::g_cr3_cache && cr3_cache::g_cr3_cache->try_get(sub_cmd.pid, sub_cmd.cr3)) {
        return physmem::runtime::copy_memory_from_constructed_cr3(
            cmd.sub_command_ptr, &sub_cmd, sizeof(sub_cmd), user_cr3);
    }

    sub_cmd.cr3 = cr3_decryption::eproc::get_cr3(sub_cmd.pid);
    
    if (!sub_cmd.cr3) {
        sub_cmd.cr3 = utility::get_cr3(sub_cmd.pid);
    }

    if (!sub_cmd.cr3) {
        PEPROCESS eproc = nullptr;
        NTSTATUS lookup_status = PsLookupProcessByProcessId((HANDLE)sub_cmd.pid, &eproc);
        if (NT_SUCCESS(lookup_status) && eproc) {
            uint64_t dtb = *(uint64_t*)((uint8_t*)eproc + 0x28);
            ObDereferenceObject(eproc);
            if (dtb) {
                sub_cmd.cr3 = dtb;
            }
        }
    }

    if (!sub_cmd.cr3) {
        status = status_cr3_not_found;
    } else {
        if (cr3_cache::g_cr3_cache) {
            cr3_cache::g_cr3_cache->insert(sub_cmd.pid, sub_cmd.cr3);
        }
        status = physmem::runtime::copy_memory_from_constructed_cr3(
            cmd.sub_command_ptr, &sub_cmd, sizeof(sub_cmd), user_cr3);
    }

    LARGE_INTEGER end_time = KeQueryPerformanceCounter(nullptr);
    uint32_t duration_us = (uint32_t)((end_time.QuadPart - start_time.QuadPart) / 10);

    if (structured_logging::g_logger) {
        structured_logging::g_logger->update_telemetry_get_cr3(duration_us, status != status_success);
    }

    return status;
}

project_status GetModuleBaseHandler::execute(const command_t& cmd, uint64_t user_cr3) {
    auto status = validators::validate_command_ptr(reinterpret_cast<uint64_t>(cmd.sub_command_ptr));
    if (status != status_success) {
        return status;
    }

    get_module_base_t sub_cmd = {0};
    status = physmem::runtime::copy_memory_to_constructed_cr3(&sub_cmd, cmd.sub_command_ptr, 
                                                              sizeof(sub_cmd), user_cr3);
    if (status != status_success) {
        return status;
    }

    if (!validators::is_valid_pid(sub_cmd.pid)) {
        return status_process_id_invalid;
    }

    status = validators::validate_module_name(sub_cmd.module_name);
    if (status != status_success) {
        return status;
    }

    PEPROCESS target_eproc = nullptr;
    NTSTATUS lookup_status = PsLookupProcessByProcessId((HANDLE)sub_cmd.pid, &target_eproc);
    if (NT_SUCCESS(lookup_status) && target_eproc) {
        const char* image_name = (const char*)PsGetProcessImageFileName(target_eproc);
        bool exact_match = image_name && (_stricmp(image_name, sub_cmd.module_name) == 0);
        bool prefix_match = image_name && (_strnicmp(sub_cmd.module_name, image_name, 15) == 0);
        if (exact_match || prefix_match) {
            sub_cmd.module_base = (uint64_t)PsGetProcessSectionBaseAddress(target_eproc);
        }
        ObDereferenceObject(target_eproc);
    }

    if (!sub_cmd.module_base) {
        sub_cmd.module_base = cr3_decryption::peb::get_module_base(sub_cmd.pid, sub_cmd.module_name);
    }

    if (!sub_cmd.module_base) {
        status = status_module_not_loaded;
    } else {
        status = physmem::runtime::copy_memory_from_constructed_cr3(
            cmd.sub_command_ptr, &sub_cmd, sizeof(sub_cmd), user_cr3);
    }

    if (structured_logging::g_logger) {
        structured_logging::g_logger->update_telemetry_get_module_base(status != status_success);
    }

    return status;
}

project_status GetModuleSizeHandler::execute(const command_t& cmd, uint64_t user_cr3) {
    auto status = validators::validate_command_ptr(reinterpret_cast<uint64_t>(cmd.sub_command_ptr));
    if (status != status_success) {
        return status;
    }

    get_module_size_t sub_cmd = {0};
    status = physmem::runtime::copy_memory_to_constructed_cr3(&sub_cmd, cmd.sub_command_ptr, 
                                                              sizeof(sub_cmd), user_cr3);
    if (status != status_success) {
        return status;
    }

    if (!validators::is_valid_pid(sub_cmd.pid)) {
        return status_process_id_invalid;
    }

    status = validators::validate_module_name(sub_cmd.module_name);
    if (status != status_success) {
        return status;
    }

    sub_cmd.module_size = cr3_decryption::peb::get_module_size(sub_cmd.pid, sub_cmd.module_name);
    
    if (!sub_cmd.module_size) {
        status = status_module_not_loaded;
    } else {
        status = physmem::runtime::copy_memory_from_constructed_cr3(
            cmd.sub_command_ptr, &sub_cmd, sizeof(sub_cmd), user_cr3);
    }

    if (structured_logging::g_logger) {
        structured_logging::g_logger->update_telemetry_get_module_size(status != status_success);
    }

    return status;
}

project_status CopyVirtualMemoryHandler::execute(const command_t& cmd, uint64_t user_cr3) {
    LARGE_INTEGER start_time = KeQueryPerformanceCounter(nullptr);

    auto status = validators::validate_command_ptr(reinterpret_cast<uint64_t>(cmd.sub_command_ptr));
    if (status != status_success) {
        return status;
    }

    copy_virtual_memory_t sub_cmd = {0};
    status = physmem::runtime::copy_memory_to_constructed_cr3(&sub_cmd, cmd.sub_command_ptr, 
                                                              sizeof(sub_cmd), user_cr3);
    if (status != status_success) {
        return status;
    }

    status = validators::validate_copy_params(sub_cmd.dst, sub_cmd.src, sub_cmd.size, 
                                              sub_cmd.dst_cr3, sub_cmd.src_cr3);
    if (status != status_success) {
        return status;
    }

    status = physmem::runtime::copy_virtual_memory(sub_cmd.dst, sub_cmd.src, sub_cmd.size, 
                                                   sub_cmd.dst_cr3, sub_cmd.src_cr3);

    LARGE_INTEGER end_time = KeQueryPerformanceCounter(nullptr);
    uint32_t duration_us = (uint32_t)((end_time.QuadPart - start_time.QuadPart) / 10);

    if (structured_logging::g_logger) {
        structured_logging::g_logger->update_telemetry_copy_memory(duration_us, sub_cmd.size, 
                                                                   status != status_success);
    }

    return status;
}

project_status GetLdrDataTableEntryCountHandler::execute(const command_t& cmd, uint64_t user_cr3) {
    auto status = validators::validate_command_ptr(reinterpret_cast<uint64_t>(cmd.sub_command_ptr));
    if (status != status_success) {
        return status;
    }

    get_ldr_data_table_entry_count_t sub_cmd = {0};
    status = physmem::runtime::copy_memory_to_constructed_cr3(&sub_cmd, cmd.sub_command_ptr, 
                                                              sizeof(sub_cmd), user_cr3);
    if (status != status_success) {
        return status;
    }

    if (!validators::is_valid_pid(sub_cmd.pid)) {
        return status_process_id_invalid;
    }

    sub_cmd.count = cr3_decryption::peb::get_data_table_entry_count(sub_cmd.pid);
    
    if (!sub_cmd.count) {
        status = status_module_enumeration_failed;
    } else {
        status = physmem::runtime::copy_memory_from_constructed_cr3(
            cmd.sub_command_ptr, &sub_cmd, sizeof(sub_cmd), user_cr3);
    }

    return status;
}

project_status GetDataTableEntryInfoHandler::execute(const command_t& cmd, uint64_t user_cr3) {
    auto status = validators::validate_command_ptr(reinterpret_cast<uint64_t>(cmd.sub_command_ptr));
    if (status != status_success) {
        return status;
    }

    cmd_get_data_table_entry_info_t sub_cmd = {0};
    status = physmem::runtime::copy_memory_to_constructed_cr3(&sub_cmd, cmd.sub_command_ptr, 
                                                              sizeof(sub_cmd), user_cr3);
    if (status != status_success) {
        return status;
    }

    if (!validators::is_valid_pid(sub_cmd.pid)) {
        return status_process_id_invalid;
    }

    if (!sub_cmd.info_array) {
        return status_invalid_parameter;
    }

    status = cr3_decryption::peb::get_data_table_entry_info(sub_cmd.pid, sub_cmd.info_array, user_cr3);
    return status;
}

project_status OutputLogsHandler::execute(const command_t& cmd, uint64_t user_cr3) {
    auto status = validators::validate_command_ptr(reinterpret_cast<uint64_t>(cmd.sub_command_ptr));
    if (status != status_success) {
        return status;
    }

    cmd_output_logs_t sub_cmd = {0};
    status = physmem::runtime::copy_memory_to_constructed_cr3(&sub_cmd, cmd.sub_command_ptr, 
                                                              sizeof(sub_cmd), user_cr3);
    if (status != status_success) {
        return status;
    }

    if (logging::g_logger) {
        logging::output_root_logs(sub_cmd.log_array, user_cr3, sub_cmd.count);
    }

    return physmem::runtime::copy_memory_from_constructed_cr3(
        cmd.sub_command_ptr, &sub_cmd, sizeof(sub_cmd), user_cr3);
}

project_status RemoveFromSystemPageTablesHandler::execute(const command_t& cmd, uint64_t user_cr3) {
    static bool is_removed = false;

    if (is_removed) {
        return status_success;
    }

    auto status = physmem::paging_manipulation::win_unmap_memory_range(
        g_driver_base, physmem::util::get_system_cr3().flags, g_driver_size);

    if (status == status_success) {
        is_removed = true;
    }

    return status;
}

project_status UnloadDriverHandler::execute(const command_t& cmd, uint64_t user_cr3) {
    return communication::unhook_data_ptr();
}

project_status PingDriverHandler::execute(const command_t& cmd, uint64_t user_cr3) {
    return status_success;
}

}
