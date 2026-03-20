# ──────────────────────────────────────────────────────
#  PDE Core — Build Script   (works from any PS window)
# ──────────────────────────────────────────────────────
$ErrorActionPreference = "Stop"

# 1. Bootstrap MSVC if cl.exe not in PATH
if (-not (Get-Command cl -ErrorAction SilentlyContinue)) {
    Write-Host "  [build] Searching for vcvarsall.bat..." -ForegroundColor DarkGray
    $vcvars = Get-ChildItem -Path "D:\Software\Visual Studio 2022" -Recurse -Filter "vcvarsall.bat" -ErrorAction SilentlyContinue | Select-Object -First 1 -ExpandProperty FullName
    if (-not $vcvars) { Write-Host "  ERROR: vcvarsall.bat not found." -ForegroundColor Red; exit 1 }
    Write-Host "  [build] Found: $vcvars" -ForegroundColor DarkGray
    $tmpBat = [System.IO.Path]::GetTempFileName() + ".bat"
    $batLines = "@echo off", "call `"$vcvars`" x64 > nul", "set"
    [System.IO.File]::WriteAllLines($tmpBat, $batLines, [System.Text.Encoding]::ASCII)
    $envDump = & cmd /c $tmpBat
    Remove-Item $tmpBat -ErrorAction SilentlyContinue
    $pat = '^([^=]+)=(.*)$'
    foreach ($ln in $envDump) { if ($ln -match $pat) { [System.Environment]::SetEnvironmentVariable($Matches[1], $Matches[2], "Process") } }
    Write-Host "  [build] MSVC ready." -ForegroundColor DarkGray
}

# 2. Output dir
if (-not (Test-Path "bin")) { New-Item -ItemType Directory -Path "bin" | Out-Null }

# 3. Sources
$sources = Get-ChildItem -Path "src" -Recurse -Filter "*.cpp" | Select-Object -ExpandProperty FullName
Write-Host ""; Write-Host "  Compiling PDE Core..." -ForegroundColor Cyan
Write-Host "  Sources : $($sources.Count) files" -ForegroundColor DarkGray; Write-Host ""

# 4. Compile
& cl /nologo /EHsc /std:c++17 /I"includes" /I"src" $sources /Fe:"bin\pde_core.exe" /Fo:"bin\" Shell32.lib Advapi32.lib

if ($LASTEXITCODE -eq 0) { Write-Host ""; Write-Host "  Build successful: bin\pde_core.exe" -ForegroundColor Green; Write-Host "" }
else { Write-Host ""; Write-Host "  Build FAILED" -ForegroundColor Red; Write-Host ""; exit 1 }