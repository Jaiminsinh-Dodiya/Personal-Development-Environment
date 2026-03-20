#include "modules/college/open_stack_command.h"
#include "core/colors.h"
#include "core/system_service.h"
#include "nlohmann/json.hpp"

#include <iostream>
#include <fstream>
#include <filesystem>

using json = nlohmann::json;

// ── Helpers ──────────────────────────────────────────────────
static void launch(std::string_view label,
                   const std::filesystem::path& path,
                   SystemService::OpenOptions opts = {}) {
    if (!SystemService::Open(path, opts))
        std::cerr << Color::RED << "Failed to launch: " << label
                  << Color::RESET << '\n';
}

// ── Construction ─────────────────────────────────────────────
OpenStackCommand::OpenStackCommand(const std::string& configPath)
    : configPath_(configPath) {}

std::string OpenStackCommand::GetName() const { return "open"; }

std::string OpenStackCommand::GetHelp() const {
    return "Open a college tech stack (e.g., ajava, cs, wordpress)";
}

// ── Execution (unchanged logic from original CollegeModule) ──
void OpenStackCommand::Execute(const std::vector<std::string>& args,
                               bool /*dryRun*/) const {
    // 1. Load the Database
    std::ifstream file(configPath_);
    if (!file.is_open()) {
        std::cerr << Color::RED << "Critical Error: Could not open "
                  << configPath_ << Color::RESET << '\n';
        return;
    }

    json db;
    file >> db;
    std::string xamppDir = db.value("xampp_dir", "");

    if (args.empty()) {
        std::cout << Color::YELLOW
                  << "Usage: pde college <stack_name>"
                  << Color::RESET << '\n';
        return;
    }

    std::string stackKey = args[0];

    // 2. Query the Stack
    if (!db["stacks"].contains(stackKey)) {
        std::cerr << Color::RED << "Unknown tech stack: \""
                  << stackKey << "\"" << Color::RESET << '\n';
        return;
    }

    auto stack = db["stacks"][stackKey];
    std::cout << Color::CYAN << Color::BOLD << "Orchestrating "
              << stack.value("label", stackKey) << "..."
              << Color::RESET << '\n';

    // 3. Dynamic Execution based on JSON structure
    if (stack.contains("path")) {
        launch("Primary Target", stack["path"].get<std::string>());
    }

    if (stack.contains("apps")) {
        for (auto& app : stack["apps"])
            launch("App", app.get<std::string>());
    }

    if (stack.contains("scripts")) {
        for (auto& script : stack["scripts"]) {
            std::filesystem::path bat =
                std::filesystem::path(xamppDir) / script.get<std::string>();
            launch(script.get<std::string>(), bat,
                   SystemService::OpenOptions::Hidden());
        }
    }

    if (stack.contains("url")) {
        launch("Web Service", stack["url"].get<std::string>());
    }
}
