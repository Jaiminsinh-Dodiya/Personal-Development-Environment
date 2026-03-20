#pragma once
#include <string>
#include <vector>
#include <memory>
#include "core/icommand.h"

// ============================================================
//  MODULE INTERFACE (Abstract Base Class)
//  Each module owns a vector of commands and is identified by
//  a short name used for CLI dispatch.
// ============================================================
class IPdeModule {
public:
    virtual ~IPdeModule() = default;

    // Short name used for CLI dispatch (e.g. "college", "ue")
    virtual std::string GetName() const = 0;

    // Human-readable description shown in help text
    virtual std::string GetDescription() const = 0;

    // All commands this module exposes.
    // The first command is treated as the default if no command
    // name is provided on the CLI.
    virtual const std::vector<std::unique_ptr<ICommand>>& GetCommands() const = 0;
};
