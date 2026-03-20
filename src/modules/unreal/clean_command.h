#pragma once
#include "core/icommand.h"

// ============================================================
//  CLEAN COMMAND  —  pde ue clean [--dry-run]
//  Deletes Binaries/, Intermediate/, Saved/, DerivedDataCache/.
//  When dryRun is true it only prints what it *would* delete.
// ============================================================
class CleanCommand : public ICommand {
public:
    std::string GetName() const override;
    std::string GetHelp() const override;
    void Execute(const std::vector<std::string>& args,
                 bool dryRun = false) const override;
};
