#include "services/project_service.h"
#include "core/colors.h"

#include <iostream>
#include <fstream>
#include <windows.h>

namespace ProjectService {

// ── Project Discovery ─────────────────────────────────────────
std::optional<std::filesystem::path> FindUProject() {
    std::error_code ec;
    for (const auto& entry : std::filesystem::directory_iterator(
             std::filesystem::current_path(), ec)) {
        if (entry.is_regular_file() && entry.path().extension() == ".uproject")
            return entry.path();
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

    // Try 32-bit registry view first (where Epic usually writes),
    // then fall back to the native 64-bit view.
    HKEY hKey = tryOpen(KEY_READ | KEY_WOW64_32KEY);
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

// ── Config Save ───────────────────────────────────────────────
bool SaveProjectConfig(const json& config) {
    std::filesystem::path pdeDir =
        std::filesystem::current_path() / PDE_DIR;

    // Create .pde/ if it doesn't exist yet
    if (!std::filesystem::exists(pdeDir)) {
        std::error_code ec;
        std::filesystem::create_directory(pdeDir, ec);
        if (ec) {
            std::cerr << Color::RED
                      << "Error: Could not create " << PDE_DIR
                      << " directory: " << ec.message()
                      << Color::RESET << '\n';
            return false;
        }
    }

    std::filesystem::path configPath = pdeDir / CONFIG_FILE;
    std::ofstream f(configPath);
    if (!f.is_open()) {
        std::cerr << Color::RED
                  << "Error: Could not write " << configPath
                  << Color::RESET << '\n';
        return false;
    }

    f << config.dump(2) << '\n';
    return true;
}

} // namespace ProjectService
