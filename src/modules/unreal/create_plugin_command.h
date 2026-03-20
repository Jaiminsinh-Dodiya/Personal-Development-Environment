#pragma once
#include "core/icommand.h"

// pde ue create plugin <PluginName>
// Creates Plugins/<PluginName>/ with .uplugin descriptor and C++ module boilerplate
class CreatePluginCommand : public ICommand {
public:
    std::string GetName() const override;
    std::string GetHelp() const override;
    void Execute(const std::vector<std::string>& args, bool dryRun) const override;
};
