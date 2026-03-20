#pragma once
#include <filesystem>
#include <string>
#include <windows.h>

// ============================================================
//  SERVICE LAYER — Win32 process launcher
// ============================================================
class SystemService {
public:
    struct OpenOptions {
        std::string arguments;
        int         showMode = SW_SHOW;
        bool        asAdmin  = false;

        static OpenOptions Hidden() {
            OpenOptions o;
            o.showMode = SW_HIDE;
            return o;
        }
    };

    // Launch a file, app, URL, or script via ShellExecuteA.
    // Returns true on success.
    static bool Open(const std::filesystem::path& path,
                     OpenOptions opts = {});
};
