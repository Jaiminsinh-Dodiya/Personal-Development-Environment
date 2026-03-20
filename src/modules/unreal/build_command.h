#pragma once
#include "core/icommand.h"

// ============================================================
//  BUILD COMMAND  —  pde ue build
//  Reads .pde/project.json and invokes UnrealBuildTool (UBT).
// ============================================================
class BuildCommand : public ICommand {
public:
    std::string GetName() const override;
    std::string GetHelp() const override;
    void Execute(const std::vector<std::string>& args,
                 bool dryRun = false) const override;
};
