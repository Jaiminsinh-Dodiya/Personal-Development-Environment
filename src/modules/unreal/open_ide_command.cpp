#include "modules/unreal/open_ide_command.h"
#include "services/project_service.h"
#include "core/system_service.h"
#include "core/colors.h"

#include <iostream>
#include <filesystem>

std::string OpenIdeCommand::GetName() const { return "open ide"; }
std::string OpenIdeCommand::GetHelp() const {
    return "Open the project solution (.sln / .slnx) in Visual Studio or Rider";
}

void OpenIdeCommand::Execute(const std::vector<std::string>& /*args*/,
                             bool dryRun) const {
    // ── 1. Find .uproject root (via directory climbing) ───────
    auto uprojectOpt = ProjectService::FindUProject();
    if (!uprojectOpt) {
        std::cerr << Color::RED
                  << "Error: Could not locate the .uproject file."
                  << Color::RESET << '\n';
        return;
    }

    std::filesystem::path projectRoot = uprojectOpt->parent_path();

    // ── 2. Find .sln or .slnx in project root ─────────────────
    std::filesystem::path solutionFile;
    std::error_code ec;
    for (const auto& entry : std::filesystem::directory_iterator(projectRoot, ec)) {
        const auto& ext = entry.path().extension();
        if (entry.is_regular_file() && (ext == ".sln" || ext == ".slnx")) {
            solutionFile = entry.path();
            break;
        }
    }

    if (solutionFile.empty()) {
        std::cerr << Color::YELLOW
                  << "Warning: No .sln or .slnx found in " << projectRoot.generic_string() << "\n"
                  << "  Generate solution files via: right-click .uproject -> "
                     "'Generate Visual Studio project files'\n"
                  << Color::RESET;
        return;
    }

    std::cout << Color::CYAN << Color::BOLD << "Opening IDE solution...\n"
              << Color::RESET
              << "  Solution: " << solutionFile.generic_string() << '\n';

    if (dryRun) {
        std::cout << Color::YELLOW << "[dry-run] Would open IDE — no action taken."
                  << Color::RESET << '\n';
        return;
    }

    // ── 3. ShellExecute opens the .sln with the OS default app ─
    // This correctly handles Visual Studio AND JetBrains Rider.
    // We do NOT use CreateProcessA here because .sln is not an .exe.
    SystemService::OpenOptions opts;
    if (SystemService::Open(solutionFile, opts)) {
        std::cout << Color::GREEN << Color::BOLD
                  << "  IDE launching..."
                  << Color::RESET << '\n';
    } else {
        std::cerr << Color::RED << "Error: Failed to open the solution file."
                  << Color::RESET << '\n';
    }
}
