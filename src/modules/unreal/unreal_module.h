#pragma once
#include "core/ipde_module.h"

// ============================================================
//  UNREAL MODULE
//  Exposes: pde ue init | build | clean
// ============================================================
class UnrealModule : public IPdeModule {
private:
    std::vector<std::unique_ptr<ICommand>> commands_;

public:
    UnrealModule();

    std::string GetName()        const override;
    std::string GetDescription() const override;
    const std::vector<std::unique_ptr<ICommand>>& GetCommands() const override;
};
