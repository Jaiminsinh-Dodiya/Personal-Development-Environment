#pragma once
#include "core/icommand.h"

// ============================================================
//  INIT COMMAND  —  pde ue init
//  Scans CWD for a .uproject, detects the UE engine from the
//  Windows Registry, and writes .pde/project.json.
// ============================================================
class InitCommand : public ICommand {
public:
    std::string GetName() const override;
    std::string GetHelp() const override;
    void Execute(const std::vector<std::string>& args,
                 bool dryRun = false) const override;
};
