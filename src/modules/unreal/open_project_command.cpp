#include "modules/unreal/open_project_command.h"
#include "modules/unreal/open_editor_command.h"
#include "modules/unreal/open_ide_command.h"
#include "core/colors.h"

#include <iostream>

std::string OpenProjectCommand::GetName() const { return "open project"; }
std::string OpenProjectCommand::GetHelp() const {
    return "Open both the Unreal Editor AND the IDE solution simultaneously";
}

void OpenProjectCommand::Execute(const std::vector<std::string>& args,
                                  bool dryRun) const {
    std::cout << Color::CYAN << Color::BOLD
              << "=== Opening Full Project (Editor + IDE) ===\n" << Color::RESET;

    // Both are DETACHED_PROCESS / ShellExecute — they spawn independently
    // and this function returns immediately after both are launched.
    std::cout << Color::CYAN << "-- Unreal Editor --\n" << Color::RESET;
    OpenEditorCommand{}.Execute(args, dryRun);

    std::cout << Color::CYAN << "-- IDE Solution --\n" << Color::RESET;
    OpenIdeCommand{}.Execute(args, dryRun);

    std::cout << Color::GREEN << Color::BOLD
              << "Both launched! This terminal is now free.\n" << Color::RESET;
}
