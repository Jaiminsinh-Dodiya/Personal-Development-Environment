#pragma once
#include "core/icommand.h"

// ============================================================
//  OPEN STACK COMMAND
//  Reads a JSON config and launches an entire dev stack
//  (apps, scripts, URLs) based on the stack key.
// ============================================================
class OpenStackCommand : public ICommand {
private:
    std::string configPath_;

public:
    explicit OpenStackCommand(const std::string& configPath);

    std::string GetName() const override;
    std::string GetHelp() const override;
    void Execute(const std::vector<std::string>& args,
                 bool dryRun = false) const override;
};
