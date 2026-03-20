#include "modules/unreal/create_plugin_command.h"
#include "services/project_service.h"
#include "core/colors.h"
#include "nlohmann/json.hpp"

#include <iostream>
#include <fstream>
#include <filesystem>

using json = nlohmann::json;

std::string CreatePluginCommand::GetName() const { return "create plugin"; }
std::string CreatePluginCommand::GetHelp() const {
    return "Scaffold a new UE plugin: create plugin <PluginName>";
}

void CreatePluginCommand::Execute(const std::vector<std::string>& args, bool dryRun) const {
    // ── 1. Validate args ──────────────────────────────────────
    if (args.empty()) {
        std::cerr << Color::RED
                  << "Usage: pde ue create plugin <PluginName>"
                  << Color::RESET << '\n';
        return;
    }
    const std::string& pluginName = args[0];

    // ── 2. Find project root ──────────────────────────────────
    auto uprojectOpt = ProjectService::FindUProject();
    if (!uprojectOpt) {
        std::cerr << Color::RED
                  << "Error: No .uproject file found. Are you in a UE project?"
                  << Color::RESET << '\n';
        return;
    }
    std::filesystem::path projectRoot = uprojectOpt->parent_path();

    // ── 3. Build directory structure ──────────────────────────
    std::filesystem::path pluginRoot   = projectRoot / "Plugins" / pluginName;
    std::filesystem::path sourceDir    = pluginRoot  / "Source"  / pluginName;
    std::filesystem::path publicDir    = sourceDir   / "Public";
    std::filesystem::path privateDir   = sourceDir   / "Private";

    std::cout << Color::CYAN << Color::BOLD << "Creating plugin: " << pluginName << '\n'
              << Color::RESET
              << "  Root   : " << pluginRoot.generic_string()  << '\n'
              << "  Public : " << publicDir.generic_string()   << '\n'
              << "  Private: " << privateDir.generic_string()  << '\n';

    if (dryRun) {
        std::cout << Color::YELLOW << "[dry-run] No files written.\n" << Color::RESET;
        return;
    }

    if (std::filesystem::exists(pluginRoot)) {
        std::cerr << Color::YELLOW << "Warning: Plugin directory already exists.\n"
                  << Color::RESET;
        return;
    }

    // ── 4. Create directories ─────────────────────────────────
    std::error_code ec;
    std::filesystem::create_directories(publicDir,  ec);
    std::filesystem::create_directories(privateDir, ec);

    // ── 5. Write .uplugin descriptor (JSON) ───────────────────
    json uplugin = {
        {"FileVersion", 3},
        {"Version",     1},
        {"VersionName", "1.0"},
        {"FriendlyName", pluginName},
        {"Description",  ""},
        {"Category",     "Other"},
        {"CreatedBy",    "PDE Core"},
        {"CanContainContent", false},
        {"IsBetaVersion",    false},
        {"IsExperimentalVersion", false},
        {"Installed",    false},
        {"Modules", json::array({
            {
                {"Name",          pluginName},
                {"Type",          "Runtime"},
                {"LoadingPhase",  "Default"}
            }
        })}
    };

    std::filesystem::path upluginPath = pluginRoot / (pluginName + ".uplugin");
    {
        std::ofstream f(upluginPath);
        f << uplugin.dump(4) << '\n';
    }

    // ── 6. Write Build.cs ─────────────────────────────────────
    std::filesystem::path buildcsPath = sourceDir / (pluginName + ".Build.cs");
    {
        std::ofstream f(buildcsPath);
        f << "using UnrealBuildTool;\n\n"
          << "public class " << pluginName << " : ModuleRules\n{\n"
          << "    public " << pluginName << "(ReadOnlyTargetRules Target) : base(Target)\n    {\n"
          << "        PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;\n\n"
          << "        PublicDependencyModuleNames.AddRange(new string[] {\n"
          << "            \"Core\",\n"
          << "            \"CoreUObject\",\n"
          << "            \"Engine\"\n"
          << "        });\n"
          << "    }\n}\n";
    }

    // ── 7. Write Module header ────────────────────────────────
    {
        std::ofstream f(publicDir / (pluginName + "Module.h"));
        f << "#pragma once\n\n"
          << "#include \"Modules/ModuleManager.h\"\n\n"
          << "class F" << pluginName << "Module : public IModuleInterface\n{\n"
          << "public:\n"
          << "    virtual void StartupModule() override;\n"
          << "    virtual void ShutdownModule() override;\n"
          << "};\n";
    }

    // ── 8. Write Module implementation ───────────────────────
    {
        std::ofstream f(privateDir / (pluginName + "Module.cpp"));
        f << "#include \"" << pluginName << "Module.h\"\n\n"
          << "#define LOCTEXT_NAMESPACE \"F" << pluginName << "Module\"\n\n"
          << "void F" << pluginName << "Module::StartupModule() {}\n\n"
          << "void F" << pluginName << "Module::ShutdownModule() {}\n\n"
          << "#undef LOCTEXT_NAMESPACE\n\n"
          << "IMPLEMENT_MODULE(F" << pluginName << "Module, " << pluginName << ")\n";
    }

    std::cout << Color::GREEN << Color::BOLD
              << "Plugin \"" << pluginName << "\" created successfully!\n"
              << Color::RESET
              << "  Next: right-click .uproject -> \"Generate Visual Studio project files\"\n";
}
