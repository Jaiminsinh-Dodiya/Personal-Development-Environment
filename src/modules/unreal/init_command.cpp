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

    // ── 3. Smart Engine Detection (with version-mismatch fallback) ──
    std::cout << Color::CYAN << "Querying registry for UE " << version << "..."
              << Color::RESET << '\n';

    std::string boundVersion = version; // The version we will actually bind to
    auto engineOpt = ProjectService::DetectEngineFromRegistry(version);

    if (!engineOpt) {
        // ── Version mismatch: scan ALL installed engines ──────
        std::cout << Color::YELLOW << Color::BOLD
                  << "  [!] UE " << version << " not found on this machine.\n"
                  << Color::RESET;

        auto allVersions = ProjectService::GetAllInstalledEngineVersions();

        if (allVersions.empty()) {
            std::cerr << Color::RED
                      << "  No Unreal Engine installations found in the registry.\n"
                      << "  Please install Unreal Engine and try again.\n"
                      << Color::RESET;
            return;
        }

        std::cout << Color::CYAN << "  Installed engine(s) found:\n" << Color::RESET;
        for (size_t i = 0; i < allVersions.size(); ++i) {
            std::cout << "    [" << (i + 1) << "] UE " << Color::BOLD
                      << allVersions[i].first << Color::RESET
                      << "  ->  " << allVersions[i].second.generic_string() << '\n';
        }
        std::cout << Color::GREEN
                  << "  Select a version to bind to [1-" << allVersions.size() << "] (or 0 to abort): "
                  << Color::RESET;

        std::string choice;
        std::getline(std::cin, choice);
        int idx = 0;
        try { idx = std::stoi(choice); } catch (...) {}

        if (idx < 1 || idx > static_cast<int>(allVersions.size())) {
            std::cout << "Initialization aborted.\n";
            return;
        }

        engineOpt   = allVersions[idx - 1].second;
        boundVersion = allVersions[idx - 1].first;
        std::cout << Color::GREEN << "  Binding to UE " << boundVersion
                  << Color::RESET << '\n';
    }

    // engineOpt is now guaranteed to be set
    if (engineOpt) {
        std::cout << Color::GREEN << "  Engine path: "
                  << Color::RESET << engineOpt->generic_string() << '\n';
    }

    // ── 4. Build config JSON (with Path Normalization) ─────────
    json config = {
        {"engine_version",       version},
        {"bound_engine_version", boundVersion},
        {"project_name",         uproject.stem().generic_string()},
        {"project_file",         uproject.filename().generic_string()}
        // engine path is resolved at runtime via registry; not stored here
        // so .pde/project.json is portable across machines
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
