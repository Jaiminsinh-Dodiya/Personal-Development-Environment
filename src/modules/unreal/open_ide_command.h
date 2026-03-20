#pragma once
#include "core/icommand.h"

// pde ue open ide — finds .sln / .slnx and opens in default IDE
class OpenIdeCommand : public ICommand {
public:
    std::string GetName() const override;
    std::string GetHelp() const override;
    void Execute(const std::vector<std::string>& args, bool dryRun) const override;
};
