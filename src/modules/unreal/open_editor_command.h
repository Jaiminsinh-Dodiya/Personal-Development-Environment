#pragma once
#include "core/icommand.h"

// pde ue open editor — launches UnrealEditor.exe for the current project
class OpenEditorCommand : public ICommand {
public:
    std::string GetName() const override;
    std::string GetHelp() const override;
    void Execute(const std::vector<std::string>& args, bool dryRun) const override;
};
