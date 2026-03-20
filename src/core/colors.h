#pragma once
#include <string_view>

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
