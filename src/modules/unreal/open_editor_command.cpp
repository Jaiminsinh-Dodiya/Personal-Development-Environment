#include "modules/unreal/open_editor_command.h"
#include "services/project_service.h"
#include "core/system_service.h"
#include "core/colors.h"

#include <iostream>
#include <filesystem>

std::string OpenEditorCommand::GetName() const { return "open editor"; }
std::string OpenEditorCommand::GetHelp() const {
    return "Launch the Unreal Editor for this project (detached, non-blocking)";
}

void OpenEditorCommand::Execute(const std::vector<std::string>& /*args*/,
                                bool dryRun) const {
    // ── 1. Load project config ────────────────────────────────
    auto cfgOpt = ProjectService::LoadProjectConfig();
    if (!cfgOpt) {
        std::cerr << Color::RED
                  << "Error: No .pde/project.json found. Run 'pde ue init' first."
                  << Color::RESET << '\n';
        return;
    }
    const auto& cfg = *cfgOpt;

    // Use bound_engine_version if present, otherwise fall back to engine_version
    std::string version = cfg.value("bound_engine_version",
                           cfg.value("engine_version", ""));
    std::string projectFile = cfg.value("project_file", "");

    if (version.empty() || projectFile.empty()) {
        std::cerr << Color::RED << "Error: .pde/project.json is missing required fields."
                  << Color::RESET << '\n';
        return;
    }

    // ── 2. Locate UnrealEditor.exe ────────────────────────────
    auto engineOpt = ProjectService::DetectEngineFromRegistry(version);
    if (!engineOpt) {
        std::cerr << Color::RED
                  << "Error: Could not find Unreal Engine " << version << " in registry."
                  << Color::RESET << '\n';
        return;
    }

    std::filesystem::path editorExe =
        *engineOpt / "Engine" / "Binaries" / "Win64" / "UnrealEditor.exe";

    // ── 3. Locate .uproject (via directory climbing) ──────────
    auto uprojectOpt = ProjectService::FindUProject();
    if (!uprojectOpt) {
        std::cerr << Color::RED
                  << "Error: Could not locate the .uproject file."
                  << Color::RESET << '\n';
        return;
    }

    std::cout << Color::CYAN << Color::BOLD << "Opening Unreal Editor...\n"
              << Color::RESET
              << "  Editor : " << editorExe.generic_string() << '\n'
              << "  Project: " << uprojectOpt->generic_string() << '\n';

    if (dryRun) {
        std::cout << Color::YELLOW << "[dry-run] Would launch Unreal Editor — no action taken."
                  << Color::RESET << '\n';
        return;
    }

    if (!std::filesystem::exists(editorExe)) {
        std::cerr << Color::RED << "Error: UnrealEditor.exe not found at:\n  "
                  << editorExe.generic_string() << Color::RESET << '\n';
        return;
    }

    // ── 4. Launch detached ────────────────────────────────────
    // Pass the .uproject path as the arguments so the editor opens the
    // correct project directly (skipping the project browser dialog).
    SystemService::OpenOptions opts;
    opts.arguments = "\"" + uprojectOpt->generic_string() + "\"";

    if (SystemService::Open(editorExe, opts)) {
        std::cout << Color::GREEN << Color::BOLD
                  << "  Unreal Editor launching in the background."
                  << Color::RESET << '\n';
    } else {
        std::cerr << Color::RED << "Error: Failed to launch Unreal Editor."
                  << Color::RESET << '\n';
    }
}
