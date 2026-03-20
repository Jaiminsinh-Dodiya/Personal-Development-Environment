#include "services/project_service.h"
#include "core/colors.h"

#include <iostream>
#include <fstream>
#include <windows.h>

namespace ProjectService {

// ── Project Discovery ─────────────────────────────────────────
// Climbs up the directory tree until it finds a .uproject file.
std::optional<std::filesystem::path> FindUProject() {
    std::filesystem::path current = std::filesystem::current_path();
    std::error_code ec;

    while (true) {
        for (const auto& entry : std::filesystem::directory_iterator(current, ec)) {
            if (entry.is_regular_file() && entry.path().extension() == ".uproject") {
                return entry.path();
            }
        }
        
        std::filesystem::path parent = current.parent_path();
        // If we've hit the root (e.g. C:\) and parent == current, stop searching.
        if (parent == current) break;
        current = parent;
    }
    return std::nullopt;
}

// ── Engine Detection via Windows Registry ─────────────────────
std::optional<std::filesystem::path> DetectEngineFromRegistry(const std::string& version) {
    std::string keyPath = "SOFTWARE\\EpicGames\\Unreal Engine\\" + version;

    // Lambda to try opening with a specific access flag
    auto tryOpen = [&](REGSAM flags) -> HKEY {
        HKEY hKey = nullptr;
        RegOpenKeyExA(HKEY_LOCAL_MACHINE, keyPath.c_str(), 0, flags, &hKey);
        return hKey;
    };

    // Try explicit 64-bit view (since UE is 64-bit), then fallback to 32-bit view
    HKEY hKey = tryOpen(KEY_READ | KEY_WOW64_64KEY);
    if (!hKey) hKey = tryOpen(KEY_READ | KEY_WOW64_32KEY);
    if (!hKey) hKey = tryOpen(KEY_READ);
    if (!hKey) return std::nullopt;

    char buffer[512] = {};
    DWORD bufSize = sizeof(buffer);
    LONG result = RegQueryValueExA(
        hKey, "InstalledDirectory", nullptr, nullptr,
        reinterpret_cast<LPBYTE>(buffer), &bufSize);
    RegCloseKey(hKey);

    if (result != ERROR_SUCCESS) return std::nullopt;
    return std::filesystem::path(buffer);
}

// ── Config Load ───────────────────────────────────────────────
std::optional<json> LoadProjectConfig() {
    std::filesystem::path configPath =
        std::filesystem::current_path() / PDE_DIR / CONFIG_FILE;

    if (!std::filesystem::exists(configPath)) return std::nullopt;

    std::ifstream f(configPath);
    if (!f.is_open()) return std::nullopt;

    try {
        json j;
        f >> j;
        return j;
    } catch (...) {
        return std::nullopt;
    }
}

// ── Config Save (Atomic Transaction) ──────────────────────────
bool SaveProjectConfigTransaction(const json& config) {
    std::filesystem::path pdeTmpDir = std::filesystem::current_path() / ".pde_tmp";

    // 1. Wipe old tmp dir if it exists from a crashed run
    if (std::filesystem::exists(pdeTmpDir)) {
        std::error_code ec;
        std::filesystem::remove_all(pdeTmpDir, ec);
    }

    std::error_code ec;
    std::filesystem::create_directory(pdeTmpDir, ec);
    if (ec) {
        std::cerr << Color::RED << "Error: Could not create .pde_tmp directory: " 
                  << ec.message() << Color::RESET << '\n';
        return false;
    }

    std::filesystem::path configPath = pdeTmpDir / CONFIG_FILE;

    // 2. Serialize memory to string (Normalize strings in config before passing here if needed)
    std::string jsonStr = config.dump(2) + "\n";

    // 3. Write to temp file safely
    std::ofstream f(configPath, std::ios::trunc);
    if (!f.is_open()) {
        std::cerr << Color::RED << "Error: Could not write temporary file " << configPath
                  << Color::RESET << '\n';
        return false;
    }
    f << jsonStr;
    f.close();

    return true;
}

// Promotes .pde_tmp to .pde
bool CommitProjectConfigTransaction() {
    std::filesystem::path pdeTmpDir = std::filesystem::current_path() / ".pde_tmp";
    std::filesystem::path pdeDir    = std::filesystem::current_path() / PDE_DIR;

    if (!std::filesystem::exists(pdeTmpDir)) {
        return false; // Nothing to commit
    }

    std::error_code ec;
    
    // 1. Delete existing .pde folder
    if (std::filesystem::exists(pdeDir)) {
        std::filesystem::remove_all(pdeDir, ec);
        if (ec) {
            std::cerr << Color::RED << "Error: Could not remove old .pde directory: "
                      << ec.message() << Color::RESET << '\n';
            return false;
        }
    }

    // 2. Rename .pde_tmp to .pde
    std::filesystem::rename(pdeTmpDir, pdeDir, ec);
    if (ec) {
        std::cerr << Color::RED << "Error: Failed to commit transaction (rename failed): "
                  << ec.message() << Color::RESET << '\n';
        return false;
    }

    return true;
}

} // namespace ProjectService
