# Advanced Unreal Engine Module — Implementation Plan

Transform the `ue` module from a basic builder into a full-fledged Unreal Engine project management powerhouse, capable of generating code, launching editors/IDEs, and handling edge cases with professional constraints.

## Branch Strategy
All work for this phase will be done on the `feature/advanced-ue-module` branch to isolate it from the stable core.

## Architectural Improvements & Professional Patterns

### 1. The "Working Directory" Trap (Directory Climbing)
**Goal:** Allow users to run `pde ue build` from deep inside a module (e.g., `MyProject/Source/MyModule`) instead of forcing them back to the project root.
- **Change:** `ProjectService::FindUProject()` will be upgraded to recursively climb the directory tree (`..`) until it finds a `.uproject` file or hits the drive root.

### 2. Registry Permission & Architecture (x64 vs x86)
**Goal:** Ensure the 64-bit Unreal Engine registry keys are always found, even if PDE is compiled as a 32-bit executable.
- **Change:** `DetectEngineFromRegistry` will explicitly use the `KEY_WOW64_64KEY` flag in `RegOpenKeyExA` to bypass Windows Registry redirectors, alongside the existing `KEY_WOW64_32KEY` fallback.

### 3. JSON "Silent Corruption" (Safe File Writing)
**Goal:** Prevent project files from being wiped to 0 KB if the CLI crashes mid-write.
- **Change:** `ProjectService::SaveProjectConfig` will use a professional atomic write pattern:
  1. Parse to `nlohmann::json`.
  2. Serialize to a string in memory.
  3. Write string to a temporary file (`project.json.tmp`).
  4. Use `std::filesystem::rename` to atomically replace the target file.

### 4. Process "Zombie" Hanging (Detached Execution)
**Goal:** Prevent the PDE CLI window from freezing while the Unreal Editor or Visual Studio is open.
- **Change:** `SystemService::Open()` will be overhauled. Instead of relying on `system()` or basic `ShellExecute`, it will use `CreateProcessA` with the `DETACHED_PROCESS` flag. This allows PDE to exit immediately while the child process lives on.

### 5. Safe Init & Overwrite Protection
**Goal:** Prevent `pde ue init` from blindly wiping an existing configuration.
- **Change:** If `.pde/project.json` already exists, `InitCommand` will prompt: *"A project configuration already exists. Do you want to re-initialize? [y/N]"*

---

## New Commands & Capabilities

### 1. Smart Init & Version Mismatches
**Goal:** Handle the scenario where a project was built for UE 5.4, but the user only has UE 5.7 installed.
- **Change:** Query the registry for *all* installed UE versions. If the project's target version is missing, prompt the user: *"Project requires 5.4, but only [5.5, 5.7] were found. Would you like to bind to 5.7 instead?"*

### 2. Build & Rebuild Commands
- **`build`**: Existing command. Invokes UBT.
- **`rebuild`**: Equivalent to running `clean` followed by `build`.

### 3. Editor & IDE Integration (Using DETACHED_PROCESS)
- **`open editor`**: Launches `UnrealEditor.exe <uproject path>`.
- **`open ide`**: Auto-detects the `.sln` or `.slnx` and launches it.
- **`open project`**: combination macro (opens BOTH editor and IDE simultaneously).

### 4. Code Generation (Plugins & Modules)
- **`create plugin <PluginName>`**: Generates `Plugins/<PluginName>.uplugin`, `Build.cs`, and standard `Public`/`Private` boilerplate.
- **`create module <ModuleName> <ModuleType> [PluginName]`**:
  - `ModuleType` options: `Runtime`, `Editor`, `Developer`.
  - Creates the standard module boilerplate.
  - Automatically safely rewrites the `.uproject`/`.uplugin` JSON array to register the new module using the atomic write pattern.

---

## Phased Rollout

| Phase | Name | What Changes |
|-------|------|-------------|
| **1** | **Core Overhaul** | Directory climbing, `DETACHED_PROCESS` in `SystemService`, `KEY_WOW64_64KEY` registry fix, and atomic JSON saving. |
| **2** | **Smart Init** | Update `.pde` init to block accidental overwrites and handle engine version mismatches interactively. |
| **3** | **Project Launchers** | Add `open editor`, `open ide`, `open project`, and `rebuild`. |
| **4** | **Code Generators** | Add `create plugin` and `create module`. |
| **5** | **New Project Scaffold (Future)** | `create unreal project <name> [-cpp]` (Out of scope for this branch). |

---

## Verification Plan

### Core Overhaul Testing
1. Navigate to `/Binaries/Win64/` and run `pde ue build` to verify directory climbing works.
2. Verify `DetectEngineFromRegistry` successfully finds the engine path.

### Process Launching
1. Run `pde ue open editor`. Verify the PDE terminal window closes immediately (i.e., you get the PS prompt back) while the UE Editor continues loading.

### Code Gen Testing
1. Run `pde ue create module Inventory Runtime`.
2. Inspect the `.uproject` file. Verify the "Modules" array has been updated correctly without data corruption.
