#include "modules/unreal/rebuild_command.h"
#include "modules/unreal/clean_command.h"
#include "modules/unreal/build_command.h"
#include "core/colors.h"

#include <iostream>

std::string RebuildCommand::GetName() const { return "rebuild"; }
std::string RebuildCommand::GetHelp() const {
    return "Clean then build the UE project (clean + build in sequence)";
}

void RebuildCommand::Execute(const std::vector<std::string>& args, bool dryRun) const {
    std::cout << Color::CYAN << Color::BOLD
              << "=== Rebuild: Clean ===\n" << Color::RESET;
    CleanCommand{}.Execute(args, dryRun);

    std::cout << Color::CYAN << Color::BOLD
              << "=== Rebuild: Build ===\n" << Color::RESET;
    BuildCommand{}.Execute(args, dryRun);
}
