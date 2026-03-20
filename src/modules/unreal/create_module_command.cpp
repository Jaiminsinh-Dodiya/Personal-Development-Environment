#include "modules/unreal/create_module_command.h"
#include "services/project_service.h"
#include "core/colors.h"
#include "nlohmann/json.hpp"

#include <iostream>
#include <fstream>
#include <filesystem>
#include <set>

using json = nlohmann::json;

std::string CreateModuleCommand::GetName() const { return "create module"; }
std::string CreateModuleCommand::GetHelp() const {
    return "Scaffold a UE module: create module <Name> <Type> [PluginName]  (Type: Runtime|Editor|Developer)";
}

// Helper: safely inject a new module entry into a JSON descriptor file
// (.uproject or .uplugin) using the atomic rename pattern.
static bool InjectModuleIntoDescriptor(
        const std::filesystem::path& descPath,
        const std::string& moduleName,
        const std::string& moduleType,
        const std::string& loadingPhase = "Default") {

    // 1. Read existing JSON
    json desc;
    {
        std::ifstream f(descPath);
        if (!f.is_open()) {
            std::cerr << Color::RED << "Error: Cannot open " << descPath.generic_string()
                      << Color::RESET << '\n';
            return false;
        }
        try { f >> desc; }
        catch (...) {
            std::cerr << Color::RED << "Error: Failed to parse JSON in "
                      << descPath.generic_string() << Color::RESET << '\n';
            return false;
        }
    }

    // 2. Ensure "Modules" key exists (Phase 4 user advice: check first!)
    if (!desc.contains("Modules") || !desc["Modules"].is_array()) {
        desc["Modules"] = json::array();
    }

    // 3. Guard against duplicate module names
    for (const auto& mod : desc["Modules"]) {
        if (mod.value("Name", "") == moduleName) {
            std::cerr << Color::YELLOW
                      << "Warning: Module \"" << moduleName
                      << "\" is already registered in " << descPath.filename().generic_string()
                      << Color::RESET << '\n';
            return false;
        }
    }

    // 4. Append new module
    desc["Modules"].push_back({
        {"Name",         moduleName},
        {"Type",         moduleType},
        {"LoadingPhase", loadingPhase}
    });

    // 5. Atomic write: serialize -> tmp -> rename
    std::filesystem::path tmpPath = descPath.parent_path() / (descPath.filename().string() + ".tmp");
    {
        std::ofstream f(tmpPath, std::ios::trunc);
        if (!f.is_open()) {
            std::cerr << Color::RED << "Error: Cannot write temp file." << Color::RESET << '\n';
            return false;
        }
        f << desc.dump(4) << '\n';
    }
    std::error_code ec;
    std::filesystem::rename(tmpPath, descPath, ec);
    if (ec) {
        // Windows fallback
        std::filesystem::remove(descPath, ec);
        std::filesystem::rename(tmpPath, descPath, ec);
    }
    return !ec;
}

void CreateModuleCommand::Execute(const std::vector<std::string>& args, bool dryRun) const {
    // ── 1. Args: <ModuleName> <ModuleType> [PluginName] ───────
    if (args.size() < 2) {
        std::cerr << Color::RED
                  << "Usage: pde ue create module <ModuleName> <ModuleType> [PluginName]\n"
                  << "  ModuleType: Runtime | Editor | Developer\n"
                  << Color::RESET;
        return;
    }
    const std::string& moduleName = args[0];
    const std::string& moduleType = args[1];
    const std::string  pluginName = args.size() >= 3 ? args[2] : "";

    // Validate type
    std::set<std::string> validTypes{"Runtime", "Editor", "Developer"};
    if (!validTypes.count(moduleType)) {
        std::cerr << Color::RED
                  << "Error: Invalid ModuleType \"" << moduleType
                  << "\". Must be Runtime, Editor, or Developer.\n"
                  << Color::RESET;
        return;
    }

    // ── 2. Find project root ──────────────────────────────────
    auto uprojectOpt = ProjectService::FindUProject();
    if (!uprojectOpt) {
        std::cerr << Color::RED << "Error: No .uproject found. Are you in a UE project?"
                  << Color::RESET << '\n';
        return;
    }
    std::filesystem::path projectRoot = uprojectOpt->parent_path();

    // ── 3. Determine where to create the module ───────────────
    std::filesystem::path moduleParentDir;
    std::filesystem::path descriptorPath; // .uplugin or .uproject to patch

    if (!pluginName.empty()) {
        // Inside a plugin
        moduleParentDir  = projectRoot / "Plugins" / pluginName / "Source" / moduleName;
        descriptorPath   = projectRoot / "Plugins" / pluginName / (pluginName + ".uplugin");
        if (!std::filesystem::exists(descriptorPath)) {
            std::cerr << Color::RED << "Error: Plugin \"" << pluginName << "\" not found.\n"
                      << "  Run: pde ue create plugin " << pluginName << " first.\n"
                      << Color::RESET;
            return;
        }
    } else {
        // Inside the project itself
        moduleParentDir = projectRoot / "Source" / moduleName;
        descriptorPath  = *uprojectOpt;
    }

    std::filesystem::path publicDir  = moduleParentDir / "Public";
    std::filesystem::path privateDir = moduleParentDir / "Private";

    std::cout << Color::CYAN << Color::BOLD << "Creating module: " << moduleName << '\n'
              << Color::RESET
              << "  Type      : " << moduleType << '\n'
              << (pluginName.empty()
                  ? "  Location  : Project Source\n"
                  : "  Plugin    : " + pluginName + "\n")
              << "  Public    : " << publicDir.generic_string()  << '\n'
              << "  Private   : " << privateDir.generic_string() << '\n'
              << "  Descriptor: " << descriptorPath.filename().generic_string() << '\n';

    if (dryRun) {
        std::cout << Color::YELLOW << "[dry-run] No files written.\n" << Color::RESET;
        return;
    }

    if (std::filesystem::exists(moduleParentDir)) {
        std::cerr << Color::YELLOW << "Warning: Module directory already exists.\n"
                  << Color::RESET;
        return;
    }

    // ── 4. Create directories & boilerplate ───────────────────
    std::error_code ec;
    std::filesystem::create_directories(publicDir,  ec);
    std::filesystem::create_directories(privateDir, ec);

    // Build.cs
    {
        std::ofstream f(moduleParentDir / (moduleName + ".Build.cs"));
        f << "using UnrealBuildTool;\n\n"
          << "public class " << moduleName << " : ModuleRules\n{\n"
          << "    public " << moduleName << "(ReadOnlyTargetRules Target) : base(Target)\n    {\n"
          << "        PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;\n\n"
          << "        PublicDependencyModuleNames.AddRange(new string[] {\n"
          << "            \"Core\",\n"
          << "            \"CoreUObject\",\n"
          << "            \"Engine\"\n"
          << "        });\n"
          << "    }\n}\n";
    }

    // Module header
    {
        std::ofstream f(publicDir / (moduleName + "Module.h"));
        f << "#pragma once\n\n"
          << "#include \"Modules/ModuleManager.h\"\n\n"
          << "class F" << moduleName << "Module : public IModuleInterface\n{\n"
          << "public:\n"
          << "    virtual void StartupModule() override;\n"
          << "    virtual void ShutdownModule() override;\n"
          << "};\n";
    }

    // Module implementation
    {
        std::ofstream f(privateDir / (moduleName + "Module.cpp"));
        f << "#include \"" << moduleName << "Module.h\"\n\n"
          << "#define LOCTEXT_NAMESPACE \"F" << moduleName << "Module\"\n\n"
          << "void F" << moduleName << "Module::StartupModule() {}\n\n"
          << "void F" << moduleName << "Module::ShutdownModule() {}\n\n"
          << "#undef LOCTEXT_NAMESPACE\n\n"
          << "IMPLEMENT_MODULE(F" << moduleName << "Module, " << moduleName << ")\n";
    }

    // ── 5. Patch the JSON descriptor (atomic) ─────────────────
    if (InjectModuleIntoDescriptor(descriptorPath, moduleName, moduleType)) {
        std::cout << Color::GREEN << Color::BOLD
                  << "Module \"" << moduleName << "\" created and registered!\n"
                  << Color::RESET
                  << "  Next: right-click .uproject -> \"Generate Visual Studio project files\"\n";
    } else {
        std::cout << Color::YELLOW
                  << "Files created, but failed to update descriptor JSON. "
                     "Please register the module manually.\n"
                  << Color::RESET;
    }
}
