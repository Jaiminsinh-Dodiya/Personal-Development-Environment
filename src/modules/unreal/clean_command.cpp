#include "modules/unreal/clean_command.h"
#include "services/project_service.h"
#include "core/colors.h"

#include <iostream>
#include <filesystem>

std::string CleanCommand::GetName() const { return "clean"; }
std::string CleanCommand::GetHelp() const {
    return "Delete build artifacts (Binaries, Intermediate, Saved, DDC). Use --dry-run to preview.";
}

void CleanCommand::Execute(const std::vector<std::string>& args,
                           bool dryRun) const {
    // Also allow --dry-run as an explicit CLI flag
    bool isDry = dryRun;
    for (const auto& a : args)
        if (a == "--dry-run" || a == "--dry") isDry = true;

    // Verify we are in a pde-initialised project
    auto cfgOpt = ProjectService::LoadProjectConfig();
    if (!cfgOpt) {
        std::cerr << Color::RED
                  << "Error: No .pde/project.json found.\n"
                  << "  Run 'pde ue init' first."
                  << Color::RESET << '\n';
        return;
    }

    const std::filesystem::path root = std::filesystem::current_path();

    // Folders that are always safe to regenerate
    const std::vector<std::string> targets = {
        "Binaries",
        "Intermediate",
        "Saved",
        "DerivedDataCache"
    };

    std::cout << Color::CYAN << Color::BOLD
              << (isDry ? "[dry-run] " : "") << "Cleaning project..."
              << Color::RESET << '\n';

    bool anyDeleted = false;
    for (const auto& dir : targets) {
        std::filesystem::path target = root / dir;
        if (!std::filesystem::exists(target)) {
            std::cout << Color::GRAY << "  skip  " << Color::RESET
                      << dir << " (not found)\n";
            continue;
        }

        if (isDry) {
            std::cout << Color::YELLOW << "  would delete  "
                      << Color::RESET << target.string() << '\n';
        } else {
            std::error_code ec;
            std::uintmax_t removed = std::filesystem::remove_all(target, ec);
            if (ec) {
                std::cerr << Color::RED << "  error  " << dir
                          << ": " << ec.message() << Color::RESET << '\n';
            } else {
                std::cout << Color::GREEN << "  deleted  " << Color::RESET
                          << dir << " (" << removed << " items)\n";
                anyDeleted = true;
            }
        }
    }

    if (!isDry && anyDeleted)
        std::cout << Color::GREEN << Color::BOLD
                  << "\nClean complete." << Color::RESET << '\n';

    if (isDry)
        std::cout << Color::YELLOW
                  << "\nRun without --dry-run to actually delete."
                  << Color::RESET << '\n';
}
