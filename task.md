# Advanced Unreal Engine Architecture — Phased Rollout

## Phase 1: Core Overhaul (Robustness & Safety) ✅
- [x] Directory Climbing: `ProjectService::FindUProject()` traverses up to drive root
- [x] KEY_WOW64_64KEY registry fix: tries 64-bit view first, then 32-bit fallback
- [x] Atomic.pde_tmp Transaction: `SaveProjectConfigTransaction` + `CommitProjectConfigTransaction`
- [x] Detached Processes: `SystemService::Open()` uses `CreateProcessA` + `DETACHED_PROCESS`
- [x] CreateProcessA mutable buffer fix: `std::vector<char>` for command string
- [x] Path normalization: all JSON paths stored with `.generic_string()` (forward slashes)
- [x] Safe Init: overwrite prompt (`[y/N]` before re-initializing existing config)

## Phase 2: Smart Init (Version Mismatch Handling)
> Query all installed UE versions. Offer fallback if the project version is missing.
- [ ] Implement `GetAllInstalledEngineVersions()` in `ProjectService`
- [ ] Update `InitCommand` to use fallback selection if target version not found
- [ ] Save `bound_engine_version` alongside `engine_version` in `project.json`

## Phase 3: Project Launchers
> spawn editor and IDE without freezing the terminal.
- [ ] `rebuild` command (clean then build)
- [ ] `open editor` command (launches `UnrealEditor.exe <uproject>`)
- [ ] `open ide` command (finds `.sln`/`.slnx` and opens)
- [ ] `open project` command (runs both in parallel)

## Phase 4: Code Generators
> Generate boilerplate C++ and JSON for modules and plugins.
- [ ] `create plugin <PluginName>` command
- [ ] `create module <ModuleName> <ModuleType> [PluginName]` command
- [ ] Safe `.uproject`/`.uplugin` JSON injection (check if "Modules" key exists first)

## Phase 5: Testing & Polish
- [ ] End-to-end verify `DETACHED_PROCESS` (CLI exits immediately)
- [ ] End-to-end verify `.pde_tmp` transaction
- [ ] Git commit & push `feature/advanced-ue-module`
