#include "modules/unreal/build_command.h"
#include "services/project_service.h"
#include "core/colors.h"
#include "core/system_service.h"

#include <iostream>
#include <filesystem>

std::string BuildCommand::GetName() const { return "build"; }
std::string BuildCommand::GetHelp() const {
    return "Build the UE project via UnrealBuildTool (reads .pde/project.json)";
}

void BuildCommand::Execute(const std::vector<std::string>& /*args*/,
                           bool dryRun) const {
    // ── 1. Load project config ────────────────────────────────
    auto cfgOpt = ProjectService::LoadProjectConfig();
    if (!cfgOpt) {
        std::cerr << Color::RED
                  << "Error: No .pde/project.json found.\n"
                  << "  Run 'pde ue init' first."
                  << Color::RESET << '\n';
        return;
    }
    const auto& cfg = *cfgOpt;

    std::string version     = cfg.value("engine_version", "");
    std::string projectName = cfg.value("project_name", "");
    std::string projectFile = cfg.value("project_file", "");

    if (version.empty() || projectName.empty() || projectFile.empty()) {
        std::cerr << Color::RED
                  << "Error: .pde/project.json is missing required fields."
                  << Color::RESET << '\n';
        return;
    }

    // ── 2. Resolve engine path from Registry ──────────────────
    auto engineOpt = ProjectService::DetectEngineFromRegistry(version);
    if (!engineOpt) {
        std::cerr << Color::RED
                  << "Error: Could not find Unreal Engine " << version
                  << " in the Windows Registry.\n"
                  << "  Is UE " << version << " installed?"
                  << Color::RESET << '\n';
        return;
    }
    const auto& engineRoot = *engineOpt;

    // ── 3. Build UBT path & project path ──────────────────────
    std::filesystem::path ubt =
        engineRoot / "Engine" / "Binaries" / "DotNET" /
        "UnrealBuildTool" / "UnrealBuildTool.exe";

    std::filesystem::path uprojectPath =
        std::filesystem::current_path() / projectFile;

    // ── 4. Print plan ─────────────────────────────────────────
    std::cout << Color::CYAN << Color::BOLD << "Building " << projectName
              << Color::RESET << " (UE " << version << ")\n"
              << "  UBT: " << ubt.string() << '\n'
              << "  Project: " << uprojectPath.string() << '\n';

    if (dryRun) {
        std::cout << Color::YELLOW
                  << "[dry-run] Would invoke UnrealBuildTool — no action taken."
                  << Color::RESET << '\n';
        return;
    }

    // ── 5. Invoke UBT ─────────────────────────────────────────
    if (!std::filesystem::exists(ubt)) {
        std::cerr << Color::RED << "Error: UBT not found at " << ubt
                  << Color::RESET << '\n';
        return;
    }

    SystemService::Open(ubt, {});
}
