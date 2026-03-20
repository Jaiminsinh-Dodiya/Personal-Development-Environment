#include "core/colors.h"
#include "core/ipde_module.h"
#include "modules/college/college_module.h"
#include "modules/unreal/unreal_module.h"

#include <iostream>
#include <sstream>
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
//  AUTO-GENERATED HELP
// ============================================================
static void printHelp(const std::vector<std::unique_ptr<IPdeModule>>& modules) {
    std::cout << Color::CYAN << Color::BOLD
              << "--- Jaimin's PDE Core ---"
              << Color::RESET << "\n\n"
              << Color::GREEN << "Usage: " << Color::RESET
              << "pde <module> [command] [args...]\n\n";

    for (const auto& module : modules) {
        std::cout << Color::YELLOW << Color::BOLD << module->GetName()
                  << Color::RESET << " - " << module->GetDescription() << '\n';

        for (const auto& cmd : module->GetCommands()) {
            std::cout << "  " << Color::CYAN << cmd->GetName() << Color::RESET;
            
            // Pad for alignment
            int padding = 15 - static_cast<int>(cmd->GetName().length());
            if (padding > 0) std::cout << std::string(padding, ' ');

            std::cout << cmd->GetHelp() << '\n';
        }
        std::cout << '\n';
    }
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
        printHelp(modules);
        return EXIT_SUCCESS;
    }

    std::string moduleName(argv[1]);
    
    // Global help flag
    if (moduleName == "--help" || moduleName == "-h") {
        printHelp(modules);
        return EXIT_SUCCESS;
    }

    std::vector<std::string> remaining(argv + 2, argv + argc);

    // ── 2. Two-Level Dispatch ────────────────────────────────
    for (const auto& module : modules) {
        if (module->GetName() != moduleName) continue;

        const auto& commands = module->GetCommands();

        // ── Multi-word command matching ───────────────────────
        // Command names can be multi-word (e.g. "open editor", "create plugin").
        // We split each command's name on spaces and try to match against
        // the start of `remaining`. We pick the LONGEST match to avoid
        // ambiguity (e.g. "open" vs "open editor").
        const ICommand* bestMatch   = nullptr;
        size_t          bestWordCount = 0;

        if (!remaining.empty()) {
            for (const auto& cmd : commands) {
                // Tokenize the command name
                std::vector<std::string> nameParts;
                std::istringstream ss(cmd->GetName());
                std::string part;
                while (ss >> part) nameParts.push_back(part);

                // Does `remaining` start with all of nameParts?
                if (nameParts.empty() || nameParts.size() > remaining.size()) continue;

                bool match = true;
                for (size_t i = 0; i < nameParts.size(); ++i) {
                    if (remaining[i] != nameParts[i]) { match = false; break; }
                }

                // Keep the longest matching command name
                if (match && nameParts.size() > bestWordCount) {
                    bestMatch     = cmd.get();
                    bestWordCount = nameParts.size();
                }
            }
        }

        if (bestMatch) {
            // Pass everything AFTER the matched command name as args
            std::vector<std::string> cmdArgs(
                remaining.begin() + static_cast<int>(bestWordCount), remaining.end());
            bestMatch->Execute(cmdArgs, false);
            return EXIT_SUCCESS;
        }

        // No command matched — show helpful error instead of silently
        // running the first command (that was causing the init bug)
        if (remaining.empty()) {
            std::cerr << Color::YELLOW
                      << "No command specified for module \"" << moduleName << "\".\n"
                      << Color::RESET;
        } else {
            std::cerr << Color::RED
                      << "Unknown command: \"" << remaining[0] << "\" in module \""
                      << moduleName << "\".\n"
                      << Color::RESET;
        }
        std::cerr << Color::CYAN << "Run 'pde --help' to see available commands.\n"
                  << Color::RESET;
        return EXIT_FAILURE;
    }

    std::cerr << Color::RED
              << "Unknown module: \"" << moduleName << "\""
              << Color::RESET << '\n';
    return EXIT_FAILURE;
}