#pragma once
#include "core/icommand.h"

// pde ue rebuild — clean then build the UE project
class RebuildCommand : public ICommand {
public:
    std::string GetName() const override;
    std::string GetHelp() const override;
    void Execute(const std::vector<std::string>& args, bool dryRun) const override;
};
