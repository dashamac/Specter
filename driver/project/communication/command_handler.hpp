#pragma once
#include "../../core/status_codes.hpp"
#include "shared_structs.hpp"

namespace command_handlers {

class ICommandHandler {
public:
    virtual ~ICommandHandler() = default;
    virtual project_status execute(const command_t& cmd, uint64_t user_cr3) = 0;
    virtual const char* name() const = 0;
};

class GetPidByNameHandler : public ICommandHandler {
public:
    project_status execute(const command_t& cmd, uint64_t user_cr3) override;
    const char* name() const override { return "GetPidByName"; }
};

class GetCR3Handler : public ICommandHandler {
public:
    project_status execute(const command_t& cmd, uint64_t user_cr3) override;
    const char* name() const override { return "GetCR3"; }
};

class GetModuleBaseHandler : public ICommandHandler {
public:
    project_status execute(const command_t& cmd, uint64_t user_cr3) override;
    const char* name() const override { return "GetModuleBase"; }
};

class GetModuleSizeHandler : public ICommandHandler {
public:
    project_status execute(const command_t& cmd, uint64_t user_cr3) override;
    const char* name() const override { return "GetModuleSize"; }
};

class CopyVirtualMemoryHandler : public ICommandHandler {
public:
    project_status execute(const command_t& cmd, uint64_t user_cr3) override;
    const char* name() const override { return "CopyVirtualMemory"; }
};

class GetLdrDataTableEntryCountHandler : public ICommandHandler {
public:
    project_status execute(const command_t& cmd, uint64_t user_cr3) override;
    const char* name() const override { return "GetLdrDataTableEntryCount"; }
};

class GetDataTableEntryInfoHandler : public ICommandHandler {
public:
    project_status execute(const command_t& cmd, uint64_t user_cr3) override;
    const char* name() const override { return "GetDataTableEntryInfo"; }
};

class OutputLogsHandler : public ICommandHandler {
public:
    project_status execute(const command_t& cmd, uint64_t user_cr3) override;
    const char* name() const override { return "OutputLogs"; }
};

class RemoveFromSystemPageTablesHandler : public ICommandHandler {
public:
    project_status execute(const command_t& cmd, uint64_t user_cr3) override;
    const char* name() const override { return "RemoveFromSystemPageTables"; }
};

class UnloadDriverHandler : public ICommandHandler {
public:
    project_status execute(const command_t& cmd, uint64_t user_cr3) override;
    const char* name() const override { return "UnloadDriver"; }
};

class PingDriverHandler : public ICommandHandler {
public:
    project_status execute(const command_t& cmd, uint64_t user_cr3) override;
    const char* name() const override { return "PingDriver"; }
};

}
