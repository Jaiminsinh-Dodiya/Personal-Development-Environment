#include <iostream>
#include <string>
#include <string_view>
#include <vector>
#include <fstream>
#include <windows.h>
#include <filesystem>
#include "nlohmann/json.hpp"

using json = nlohmann::json;

// ============================================================
//  AESTHETIC LAYER (ANSI COLORS)
// ============================================================
namespace Color {
    constexpr std::string_view RESET   = "\033[0m";
    constexpr std::string_view BOLD    = "\033[1m";
    constexpr std::string_view RED     = "\033[31m";
    constexpr std::string_view GREEN   = "\033[32m";
    constexpr std::string_view YELLOW  = "\033[33m";
    constexpr std::string_view BLUE    = "\033[34m";
    constexpr std::string_view MAGENTA = "\033[35m";
    constexpr std::string_view CYAN    = "\033[36m";
}

// ============================================================
//  SERVICE LAYER
// ============================================================
class SystemService {
public:
    struct OpenOptions {
        std::string arguments;
        int         showMode = SW_SHOW;
        bool        asAdmin  = false;

        static OpenOptions Hidden()  { OpenOptions o; o.showMode = SW_HIDE; return o; }
    };

    static bool Open(const std::filesystem::path& path, OpenOptions opts = {}) {
        const std::string verb = opts.asAdmin ? "runas" : "open";
        const std::string dir  = path.has_parent_path() ? path.parent_path().string() : "";

        HINSTANCE result = ShellExecuteA(
            NULL, verb.c_str(), path.string().c_str(),
            opts.arguments.empty() ? NULL : opts.arguments.c_str(),
            dir.empty() ? NULL : dir.c_str(),
            opts.showMode
        );

        return reinterpret_cast<intptr_t>(result) > 32;
    }
};

// ============================================================
//  COLLEGE MODULE (DATA-DRIVEN)
// ============================================================
class CollegeModule {
private:
    const std::string CONFIG_PATH = "D:\\PDE\\config\\college.json";

    static void launch(std::string_view label, const std::filesystem::path& path, SystemService::OpenOptions opts = {}) {
        if (!SystemService::Open(path, opts))
            std::cerr << Color::RED << "✖ Failed to launch: " << label << Color::RESET << '\n';
    }

public:
    void HandleCommand(const std::vector<std::string>& args) const {
        // 1. Load the Database
        std::ifstream file(CONFIG_PATH);
        if (!file.is_open()) {
            std::cerr << Color::RED << "✖ Critical Error: Could not open " << CONFIG_PATH << Color::RESET << '\n';
            return;
        }

        json db;
        file >> db;
        std::string xamppDir = db.value("xampp_dir", "");

        if (args.empty()) {
            std::cout << Color::YELLOW << "Usage: open college [stack_name]" << Color::RESET << '\n';
            return;
        }

        std::string stackKey = args[0];

        // 2. Query the Stack
        if (!db["stacks"].contains(stackKey)) {
            std::cerr << Color::RED << "✖ Unknown tech stack: \"" << stackKey << "\"" << Color::RESET << '\n';
            return;
        }

        auto stack = db["stacks"][stackKey];
        std::cout << Color::CYAN << Color::BOLD << "🚀 Orchestrating " << stack.value("label", stackKey) << "..." << Color::RESET << '\n';

        // 3. Dynamic Execution based on JSON structure
        if (stack.contains("path")) {
            launch("Primary Target", stack["path"].get<std::string>());
        }

        if (stack.contains("apps")) {
            for (auto& app : stack["apps"]) launch("App", app.get<std::string>());
        }

        if (stack.contains("scripts")) {
            for (auto& script : stack["scripts"]) {
                std::filesystem::path bat = std::filesystem::path(xamppDir) / script.get<std::string>();
                launch(script.get<std::string>(), bat, SystemService::OpenOptions::Hidden());
            }
        }

        if (stack.contains("url")) {
            launch("Web Service", stack["url"].get<std::string>());
        }
    }
};

// ============================================================
//  MAIN DISPATCHER
// ============================================================
static void enableAnsiColors() {
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hOut == INVALID_HANDLE_VALUE) return;
    DWORD dwMode = 0;
    if (GetConsoleMode(hOut, &dwMode))
        SetConsoleMode(hOut, dwMode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
}

int main(int argc, char* argv[]) {
    enableAnsiColors();

    if (argc < 2) {
        std::cout << Color::CYAN << Color::BOLD << "--- Jaimin's PDE Core ---" << Color::RESET << '\n'
                  << Color::GREEN << "Usage: " << Color::RESET << "pde <category> [args...]\n";
        return EXIT_FAILURE;
    }

    std::string category(argv[1]);
    std::vector<std::string> args(argv + 2, argv + argc);

    if (category == "college") {
        CollegeModule{}.HandleCommand(args);
    } else {
        std::cerr << Color::RED << "✖ Unknown category: \"" << category << "\"" << Color::RESET << '\n';
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}