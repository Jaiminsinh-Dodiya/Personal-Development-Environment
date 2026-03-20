#pragma once
#include "core/icommand.h"

// pde ue create module <ModuleName> <ModuleType> [PluginName]
// ModuleType: Runtime | Editor | Developer
// If PluginName is omitted, creates the module in the project's Source/ folder
class CreateModuleCommand : public ICommand {
public:
    std::string GetName() const override;
    std::string GetHelp() const override;
    void Execute(const std::vector<std::string>& args, bool dryRun) const override;
};
