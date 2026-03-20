#include "core/system_service.h"

bool SystemService::Open(const std::filesystem::path& path, OpenOptions opts) {
    const std::string verb = opts.asAdmin ? "runas" : "open";
    const std::string dir  = path.has_parent_path()
                                 ? path.parent_path().string()
                                 : "";

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
