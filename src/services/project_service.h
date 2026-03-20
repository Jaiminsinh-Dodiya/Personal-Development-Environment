#pragma once
#include "nlohmann/json.hpp"

#include <filesystem>
#include <optional>
#include <string>
#include <vector>
#include <utility>

using json = nlohmann::json;

// ============================================================
//  PROJECT SERVICE
//  Handles all .pde/ folder discovery and management.
//  Acts as the "pde init" analogue — like git but for PDE.
// ============================================================
namespace ProjectService {

    // Directory / file name constants
    constexpr auto PDE_DIR     = ".pde";
    constexpr auto CONFIG_FILE = "project.json";

    // ── Project Discovery ─────────────────────────────────────

    // Scan CWD for a .uproject file.
    // Returns its full path if found, nullopt otherwise.
    std::optional<std::filesystem::path> FindUProject();

    // ── Engine Detection ──────────────────────────────────────

    // Auto-detect the Unreal Engine install path from the Windows Registry.
    // Reads: HKLM\SOFTWARE\EpicGames\Unreal Engine\<version>\InstalledDirectory
    // Returns the engine root path if found, nullopt otherwise.
    std::optional<std::filesystem::path> DetectEngineFromRegistry(const std::string& version);

    // Returns a list of { version_string, install_path } pairs for every UE
    // version currently installed on this machine (read from registry).
    // Returns an empty vector if none are found.
    std::vector<std::pair<std::string, std::filesystem::path>> GetAllInstalledEngineVersions();

    // ── Config I/O ────────────────────────────────────────────

    // Load .pde/project.json from CWD. Returns nullopt if missing or invalid.
    std::optional<json> LoadProjectConfig();

    // Write json to .pde_tmp/project.json in CWD (creates .pde_tmp/ if needed).
    // Returns true on success. This is part of the transaction pattern.
    bool SaveProjectConfigTransaction(const json& config);

    // Promotes .pde_tmp to .pde, fully committing the initialization.
    // Automatically cleans up any old .pde folder safely.
    bool CommitProjectConfigTransaction();

} // namespace ProjectService
