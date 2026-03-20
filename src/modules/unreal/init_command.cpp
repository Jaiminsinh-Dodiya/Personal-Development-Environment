#include "modules/unreal/init_command.h"
#include "services/project_service.h"
#include "core/colors.h"

#include <iostream>
#include <string>

std::string InitCommand::GetName() const { return "init"; }
std::string InitCommand::GetHelp() const {
    return "Initialise a .pde/project.json for the UE project in CWD";
}

void InitCommand::Execute(const std::vector<std::string>& /*args*/,
                          bool dryRun) const {
    // ── 0. Prevent Accidental Overwrite ───────────────────────
    if (ProjectService::LoadProjectConfig().has_value()) {
        std::cout << Color::YELLOW
                  << "A project configuration already exists. Do you want to re-initialize? [y/N]: "
                  << Color::RESET;
        std::string ans;
        std::getline(std::cin, ans);
        if (ans != "y" && ans != "Y") {
            std::cout << "Initialization aborted.\n";
            return;
        }
    }

    // ── 1. Find .uproject ─────────────────────────────────────
    auto uprojectOpt = ProjectService::FindUProject();
    if (!uprojectOpt) {
        std::cerr << Color::RED
                  << "Error: No .uproject file found in the current directory.\n"
                  << "  cd into your Unreal project folder and try again."
                  << Color::RESET << '\n';
        return;
    }
    const auto& uproject = *uprojectOpt;

    std::cout << Color::CYAN << "Found project: "
              << Color::BOLD << uproject.filename().string()
              << Color::RESET << '\n';

    // ── 2. Ask for engine version ──────────────────────────────
    std::cout << Color::GREEN << "Engine version (e.g. 5.3): "
              << Color::RESET;
    std::string version;
    std::getline(std::cin, version);

    if (version.empty()) {
        std::cerr << Color::RED << "Error: Engine version cannot be empty."
                  << Color::RESET << '\n';
        return;
    }

    // ── 3. Detect engine path from Registry ───────────────────
    std::cout << Color::CYAN << "Querying registry for UE " << version << "..."
              << Color::RESET << '\n';

    auto engineOpt = ProjectService::DetectEngineFromRegistry(version);

    std::string enginePathStr;
    if (engineOpt) {
        enginePathStr = engineOpt->string();
        std::cout << Color::GREEN << "  Detected engine: "
                  << Color::RESET << enginePathStr << '\n';
    } else {
        std::cout << Color::YELLOW
                  << "  Registry lookup failed. Enter engine path manually: "
                  << Color::RESET;
        std::getline(std::cin, enginePathStr);
        if (enginePathStr.empty()) {
            std::cerr << Color::RED << "Error: Engine path cannot be empty."
                      << Color::RESET << '\n';
            return;
        }
    }

    // ── 4. Build config JSON (with Path Normalization) ─────────
    json config = {
        {"engine_version", version},
        {"project_name",   uproject.stem().generic_string()},
        {"project_file",   uproject.filename().generic_string()}
        // Note: engine path is intentionally NOT stored — it is
        // resolved at runtime via the registry so the config is
        // portable across machines with different drive letters.
    };

    // ── 5. Write (or dry-run) ─────────────────────────────────
    if (dryRun) {
        std::cout << Color::YELLOW << "[dry-run] Would write .pde/project.json:\n"
                  << Color::RESET << config.dump(2) << '\n';
        return;
    }

    if (ProjectService::SaveProjectConfigTransaction(config)) {
        if (ProjectService::CommitProjectConfigTransaction()) {
            std::cout << Color::GREEN << Color::BOLD
                      << "  .pde/project.json created successfully."
                      << Color::RESET << '\n';
        } else {
            std::cerr << Color::RED << "Error: Failed to commit .pde transaction."
                      << Color::RESET << '\n';
        }
    }
}
