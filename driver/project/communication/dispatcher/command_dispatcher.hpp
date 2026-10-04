#pragma once
#include "../handlers/command_handler.hpp"
#include "../shared_structs.hpp"
#include "../../core/status_codes.hpp"

namespace command_dispatcher {

class CommandDispatcher {
private:
    command_handlers::ICommandHandler* handlers[16];
    size_t handler_count;

public:
    CommandDispatcher() : handler_count(0) {
        memset(handlers, 0, sizeof(handlers));
    }

    ~CommandDispatcher() {
        for (size_t i = 0; i < handler_count; ++i) {
            delete handlers[i];
        }
    }

    void register_handler(command_handlers::ICommandHandler* handler) {
        if (handler_count < 16) {
            handlers[handler_count++] = handler;
        }
    }

    project_status dispatch(const command_t& cmd, uint64_t user_cr3) {
        for (size_t i = 0; i < handler_count; ++i) {
            if (handlers[i]) {
                return handlers[i]->execute(cmd, user_cr3);
            }
        }

        return status_command_invalid_type;
    }
};

inline CommandDispatcher* g_dispatcher = nullptr;

inline project_status initialize_dispatcher() {
    if (g_dispatcher) {
        return status_success;
    }

    g_dispatcher = new CommandDispatcher();
    if (!g_dispatcher) {
        return status_memory_allocation_failed;
    }

    g_dispatcher->register_handler(new command_handlers::GetPidByNameHandler());
    g_dispatcher->register_handler(new command_handlers::GetCR3Handler());
    g_dispatcher->register_handler(new command_handlers::GetModuleBaseHandler());
    g_dispatcher->register_handler(new command_handlers::GetModuleSizeHandler());
    g_dispatcher->register_handler(new command_handlers::CopyVirtualMemoryHandler());
    g_dispatcher->register_handler(new command_handlers::GetLdrDataTableEntryCountHandler());
    g_dispatcher->register_handler(new command_handlers::GetDataTableEntryInfoHandler());
    g_dispatcher->register_handler(new command_handlers::OutputLogsHandler());
    g_dispatcher->register_handler(new command_handlers::RemoveFromSystemPageTablesHandler());
    g_dispatcher->register_handler(new command_handlers::UnloadDriverHandler());
    g_dispatcher->register_handler(new command_handlers::PingDriverHandler());

    return status_success;
}

inline void cleanup_dispatcher() {
    if (g_dispatcher) {
        delete g_dispatcher;
        g_dispatcher = nullptr;
    }
}

}
