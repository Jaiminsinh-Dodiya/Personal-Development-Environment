#pragma once
#include <string>
#include <vector>

// ============================================================
//  COMMAND INTERFACE (Abstract Base Class)
//  Every action (open, init, build, clean) becomes a command.
// ============================================================
class ICommand {
public:
    virtual ~ICommand() = default;

    // Short name used for CLI dispatch (e.g. "open", "build")
    virtual std::string GetName() const = 0;

    // One-line description shown in help text
    virtual std::string GetHelp() const = 0;

    // Execute the command. When dryRun is true, print what
    // *would* happen without actually doing it.
    virtual void Execute(const std::vector<std::string>& args,
                         bool dryRun = false) const = 0;
};
