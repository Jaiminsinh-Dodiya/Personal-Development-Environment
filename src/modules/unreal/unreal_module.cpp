#include "modules/unreal/unreal_module.h"
#include "modules/unreal/init_command.h"
#include "modules/unreal/build_command.h"
#include "modules/unreal/rebuild_command.h"
#include "modules/unreal/clean_command.h"
#include "modules/unreal/open_editor_command.h"
#include "modules/unreal/open_ide_command.h"
#include "modules/unreal/open_project_command.h"

UnrealModule::UnrealModule() {
    commands_.push_back(std::make_unique<InitCommand>());
    commands_.push_back(std::make_unique<BuildCommand>());
    commands_.push_back(std::make_unique<RebuildCommand>());
    commands_.push_back(std::make_unique<CleanCommand>());
    commands_.push_back(std::make_unique<OpenEditorCommand>());
    commands_.push_back(std::make_unique<OpenIdeCommand>());
    commands_.push_back(std::make_unique<OpenProjectCommand>());
}

std::string UnrealModule::GetName()        const { return "ue"; }
std::string UnrealModule::GetDescription() const {
    return "Unreal Engine project management (init, build, rebuild, clean, open editor/ide/project)";
}

const std::vector<std::unique_ptr<ICommand>>& UnrealModule::GetCommands() const {
    return commands_;
}
