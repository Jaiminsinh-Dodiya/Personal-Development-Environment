#include "core/colors.h"
#include "core/ipde_module.h"
#include "modules/college/college_module.h"
#include "modules/unreal/unreal_module.h"

#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <windows.h>

// ============================================================
//  ANSI COLOR SUPPORT
// ============================================================
static void enableAnsiColors() {
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hOut == INVALID_HANDLE_VALUE) return;
    DWORD dwMode = 0;
    if (GetConsoleMode(hOut, &dwMode))
        SetConsoleMode(hOut, dwMode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
}

// ============================================================
//  MAIN — Two-Level Dispatcher (Module → Command)
// ============================================================
int main(int argc, char* argv[]) {
    enableAnsiColors();

    // ── 1. Module Registry ───────────────────────────────────
    std::vector<std::unique_ptr<IPdeModule>> modules;
    modules.push_back(std::make_unique<CollegeModule>());
    modules.push_back(std::make_unique<UnrealModule>());

    if (argc < 2) {
        std::cout << Color::CYAN << Color::BOLD
                  << "--- Jaimin's PDE Core ---"
                  << Color::RESET << '\n'
                  << Color::GREEN << "Usage: " << Color::RESET
                  << "pde <module> [command] [args...]\n";
        return EXIT_FAILURE;
    }

    std::string moduleName(argv[1]);
    std::vector<std::string> remaining(argv + 2, argv + argc);

    // ── 2. Two-Level Dispatch ────────────────────────────────
    for (const auto& module : modules) {
        if (module->GetName() != moduleName) continue;

        const auto& commands = module->GetCommands();

        // Try to match argv[2] against a command name
        if (!remaining.empty()) {
            for (const auto& cmd : commands) {
                if (cmd->GetName() == remaining[0]) {
                    // Matched! Pass argv[3:] as args to the command.
                    std::vector<std::string> cmdArgs(
                        remaining.begin() + 1, remaining.end());
                    cmd->Execute(cmdArgs);
                    return EXIT_SUCCESS;
                }
            }
        }

        // No command matched — fall through to the default
        // (first) command with ALL remaining args.
        if (!commands.empty()) {
            commands[0]->Execute(remaining);
            return EXIT_SUCCESS;
        }

        std::cerr << Color::RED
                  << "Module \"" << moduleName << "\" has no commands."
                  << Color::RESET << '\n';
        return EXIT_FAILURE;
    }

    std::cerr << Color::RED
              << "Unknown module: \"" << moduleName << "\""
              << Color::RESET << '\n';
    return EXIT_FAILURE;
}