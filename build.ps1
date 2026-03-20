# ──────────────────────────────────────────────────────────────
#  PDE Core — Build Script
#  Compiles all .cpp source files into bin\pde_core.exe
# ──────────────────────────────────────────────────────────────

$ErrorActionPreference = "Stop"

# Ensure output directory exists
if (-not (Test-Path "bin")) {
    New-Item -ItemType Directory -Path "bin" | Out-Null
}

# Gather every .cpp under src/
$sources = Get-ChildItem -Path "src" -Recurse -Filter "*.cpp" |
           Select-Object -ExpandProperty FullName

Write-Host ""
Write-Host "  Compiling PDE Core..." -ForegroundColor Cyan
Write-Host "  Sources : $($sources.Count) files" -ForegroundColor DarkGray
Write-Host ""

# Compile with MSVC
& cl /nologo /EHsc /std:c++17 `
    /I"includes" /I"src" `
    $sources `
    /Fe:"bin\pde_core.exe" `
    /Fo:"bin\" `
    Shell32.lib

if ($LASTEXITCODE -eq 0) {
    Write-Host ""
    Write-Host "  Build successful: bin\pde_core.exe" -ForegroundColor Green
    Write-Host ""
} else {
    Write-Host ""
    Write-Host "  Build FAILED" -ForegroundColor Red
    Write-Host ""
    exit 1
}
