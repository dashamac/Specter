#include "project/project_api.hpp"
#include "project/offset_resolver.hpp"
#include "project/core/status_codes.hpp"
#include "project/cr3_cache.hpp"
#include "project/structured_logging.hpp"
#include "project/communication/command_dispatcher.hpp"
#include "project/tests.hpp"

extern "C" NTKERNELAPI ULONG PsGetProcessSessionId(_In_ PEPROCESS Process);

NTSTATUS driver_entry(void* driver_base, uint64_t loader_offsets_block_ptr) {
    project_log_success("Driver loaded via mapper at %p", driver_base);

    if (!driver_base) {
        project_log_error("Invalid driver_base parameter");
        return STATUS_UNSUCCESSFUL;
    }

    g_driver_base = driver_base;
    g_target_session_id = PsGetProcessSessionId(PsGetCurrentProcess());
    project_log_info("Driver loader session_id=%lu", g_target_session_id);

    auto loader_offsets_block = reinterpret_cast<loader_offsets::block*>(loader_offsets_block_ptr);

    NTSTATUS nt_status = offset_resolver::initialize_offsets_from_mapper(loader_offsets_block, loader_offsets_block->driver_size);
    if (!NT_SUCCESS(nt_status)) {
        project_log_error("Failed to initialize offsets from mapper with status 0x%X", nt_status);
        project_log_error("Driver REQUIRES loading via: mapper.exe roseware_driver.sys");
        return STATUS_UNSUCCESSFUL;
    }

    project_status status = status_success;

    status = physmem::init_physmem();
    if (status != status_success) {
        project_log_error("Failed to init physmem with status 0x%X", status);
        return STATUS_UNSUCCESSFUL;
    }

    status = cr3_decryption::init_eac_cr3_decryption();
    if (status != status_success) {
        project_log_error("Failed to init CR3 decryption with status 0x%X", status);
        return STATUS_UNSUCCESSFUL;
    }

    status = logging::init_root_logger();
    if (status != status_success) {
        project_log_error("Failed to init logger with status 0x%X", status);
        return STATUS_UNSUCCESSFUL;
    }

    status = (project_status)cr3_cache::initialize_cache();
    if (status != status_success) {
        project_log_error("Failed to init CR3 cache with status 0x%X", status);
        return STATUS_UNSUCCESSFUL;
    }

    status = (project_status)structured_logging::initialize_logger();
    if (status != status_success) {
        project_log_error("Failed to init structured logger with status 0x%X", status);
        return STATUS_UNSUCCESSFUL;
    }

    status = (project_status)command_dispatcher::initialize_dispatcher();
    if (status != status_success) {
        project_log_error("Failed to init command dispatcher with status 0x%X", status);
        return STATUS_UNSUCCESSFUL;
    }

    status = communication::init_communication(driver_base, g_driver_size);
    if (status != status_success) {
        project_log_error("Failed to init communication with status 0x%X", status);
        return STATUS_UNSUCCESSFUL;
    }

#ifdef DEBUG_MODE
    unit_tests::TestRunner tests;
    tests.run_all_tests();
#endif

    project_log_success("Driver initialization finished successfully");

    return STATUS_SUCCESS;
}
