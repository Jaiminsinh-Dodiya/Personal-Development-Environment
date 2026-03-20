# PDE Architecture Overhaul — Phased Rollout

## Phase 1: "The Split" — Multi-file layout + build script
> Split [pde_core.cpp](file:///d:/PDE/src/pde_core.cpp) → separate files. Build still works. Behavior unchanged.
- [x] Create `src/core/colors.h`
- [x] Create `src/core/system_service.h` + `.cpp`
- [x] Create `src/core/ipde_module.h`
- [x] Create `src/core/icommand.h`
- [x] Refactor CollegeModule → `src/modules/college/` (module + command)
- [x] Rewrite `main.cpp` with two-level dispatcher (module → command)
- [x] Create `build.ps1` (auto-finds VS 2022 from any PS window)
- [x] Verify: builds cleanly (4 sources, zero errors)
- [x] Git commit & push: `feat: multi-file plugin architecture`

## Phase 2: "The Brain" — Project-awareness + ProjectService
> Add `pde init`-style context. `.pde/` folder, project discovery.
- [x] Create `src/services/project_service.h` + `.cpp`
- [x] UE registry auto-detection logic (RegOpenKeyExA, 32+64-bit views)
- [x] Verify: builds cleanly (5 sources, zero errors)
- [x] Git commit & push: `feat: project-aware context service`

## Phase 3: "The Unreal Module" — ue init / build / clean
> Wire up the Unreal commands using ProjectService.
- [ ] Create `src/modules/unreal/unreal_module.h` + [.cpp](file:///d:/PDE/src/pde_core.cpp)
- [ ] Create `init_command`, `build_command`, `clean_command`
- [ ] Dry-run support for `clean`
- [ ] Verify: full `pde ue init` → `pde ue clean` flow
- [ ] Git commit & push: `feat: Unreal Engine module`

## Phase 4: "The Polish" — Help system + PowerShell profile
> Auto-generated help, updated shell integration.
- [ ] Auto-generated `--help` from module/command metadata
- [ ] Update [Microsoft.PowerShell_profile.ps1](file:///d:/PDE/Microsoft.PowerShell_profile.ps1)
- [ ] Final build + full regression test
- [ ] Git commit & push: `feat: help system + shell integration`
