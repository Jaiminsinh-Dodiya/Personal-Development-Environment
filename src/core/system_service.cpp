#include "core/system_service.h"

#include "core/system_service.h"

bool SystemService::Open(const std::filesystem::path& path, OpenOptions opts) {
    std::string dir = path.has_parent_path() ? path.parent_path().string() : "";
    
    // ── 1. CreateProcessA for Executables (Detached) ──
    if (path.extension() == ".exe" || path.extension() == ".bat" || path.extension() == ".cmd") {
        std::string cmd = "\"" + path.string() + "\" " + opts.arguments;
        
        // CreateProcessA requires a mutable string buffer for the command line.
        // If we pass std::string::data() directly, it might access violate.
        std::vector<char> cmdBuffer(cmd.begin(), cmd.end());
        cmdBuffer.push_back('\0');
        
        STARTUPINFOA si;
        PROCESS_INFORMATION pi;
        ZeroMemory(&si, sizeof(si));
        si.cb = sizeof(si);
        ZeroMemory(&pi, sizeof(pi));

        // Use DETACHED_PROCESS so the child completely breaks free of the CLI console
        DWORD creationFlags = DETACHED_PROCESS;

        if (CreateProcessA(
                NULL,                       // Application Name
                cmdBuffer.data(),           // Command Line (MUTABLE BUFFER)
                NULL, NULL,                 // Process/Thread attributes
                FALSE,                      // Inherit handles
                creationFlags,              // Creation flags
                NULL,                       // Environment
                dir.empty() ? NULL : dir.c_str(), // Current directory
                &si,                        // Startup info
                &pi                         // Process info
            )) {
            // Success! Close handles as we don't need to interact with the child.
            CloseHandle(pi.hProcess);
            CloseHandle(pi.hThread);
            return true;
        }
        // If CreateProcessA fails, fall through to ShellExecuteA just in case.
    }

    // ── 2. ShellExecuteA for URLs, Documents, Admin, etc ──
    const std::string verb = opts.asAdmin ? "runas" : "open";
    HINSTANCE result = ShellExecuteA(
        NULL,
        verb.c_str(),
        path.string().c_str(),
        opts.arguments.empty() ? NULL : opts.arguments.c_str(),
        dir.empty()            ? NULL : dir.c_str(),
        opts.showMode
    );

    return reinterpret_cast<intptr_t>(result) > 32;
}
