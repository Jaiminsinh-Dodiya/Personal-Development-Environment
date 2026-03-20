#pragma once
#include "core/ipde_module.h"

// ============================================================
//  COLLEGE MODULE
//  Orchestrates college development stacks from a JSON config.
// ============================================================
class CollegeModule : public IPdeModule {
private:
    std::vector<std::unique_ptr<ICommand>> commands_;

public:
    CollegeModule();

    std::string GetName() const override;
    std::string GetDescription() const override;
    const std::vector<std::unique_ptr<ICommand>>& GetCommands() const override;
};
