#include "modules/unreal/unreal_module.h"
#include "modules/unreal/init_command.h"
#include "modules/unreal/build_command.h"
#include "modules/unreal/clean_command.h"

UnrealModule::UnrealModule() {
    commands_.push_back(std::make_unique<InitCommand>());
    commands_.push_back(std::make_unique<BuildCommand>());
    commands_.push_back(std::make_unique<CleanCommand>());
}

std::string UnrealModule::GetName()        const { return "ue"; }
std::string UnrealModule::GetDescription() const { return "Unreal Engine project management (init, build, clean)"; }

const std::vector<std::unique_ptr<ICommand>>& UnrealModule::GetCommands() const {
    return commands_;
}
