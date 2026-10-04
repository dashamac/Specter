#include "handler.cpp"
#include "command_dispatcher.hpp"
#include "../validators.hpp"
#include "../integrity_check.hpp"
#include "../structured_logging.hpp"

extern "C" __int64 __fastcall handler_v2(uint64_t hwnd, uint32_t flags, ULONG_PTR dw_data) {
    if (hwnd < 0x1000) {
        return status_data_ptr_invalid;
    }

    auto cmd_ptr_status = validators::validate_command_ptr(hwnd);
    if (cmd_ptr_status != status_success) {
        return cmd_ptr_status;
    }

    uint64_t user_cr3 = shellcode::get_current_user_cr3();
    if (!user_cr3) {
        return status_cr3_invalid_format;
    }

    if (!validators::is_valid_cr3(user_cr3)) {
        return status_cr3_invalid_format;
    }

    command_t* cmd = (command_t*)ExAllocatePoolWithTag(NonPagedPoolNx, sizeof(command_t), 'CMD2');
    if (!cmd) {
        return status_memory_allocation_failed;
    }

    auto copy_status = physmem::runtime::copy_memory_to_constructed_cr3(
        cmd, (void*)hwnd, sizeof(command_t), user_cr3);

    if (copy_status != status_success) {
        ExFreePoolWithTag(cmd, 'CMD2');
        return copy_status;
    }

    if (cmd->checksum != 0) {
        auto checksum_status = integrity::validate_command_checksum(
            cmd, offsetof(command_t, checksum), cmd->checksum);
        if (checksum_status != status_success) {
            ExFreePoolWithTag(cmd, 'CMD2');
            return status_command_checksum_mismatch;
        }
    }

    project_status status = status_success;

    if (command_dispatcher::g_dispatcher) {
        status = command_dispatcher::g_dispatcher->dispatch(*cmd, user_cr3);
    } else {
        status = status_command_invalid_type;
    }

    cmd->status = (status == status_success);

    auto write_status = physmem::runtime::copy_memory_from_constructed_cr3(
        (void*)hwnd, cmd, sizeof(command_t), user_cr3);

    ExFreePoolWithTag(cmd, 'CMD2');

    return write_status != status_success ? write_status : status;
}
