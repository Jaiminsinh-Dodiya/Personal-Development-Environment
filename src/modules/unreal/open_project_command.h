#pragma once
#include "core/icommand.h"

// pde ue open project — launches both UnrealEditor.exe and the IDE simultaneously
class OpenProjectCommand : public ICommand {
public:
    std::string GetName() const override;
    std::string GetHelp() const override;
    void Execute(const std::vector<std::string>& args, bool dryRun) const override;
};
