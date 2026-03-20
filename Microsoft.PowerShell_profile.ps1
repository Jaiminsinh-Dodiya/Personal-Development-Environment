# ──────────────────────────────────────────────────────────────
#  PDE Core — Global Shell Integration
# ──────────────────────────────────────────────────────────────

# 1. Native access: `pde`
function pde {
    $exe = "D:\PDE\bin\pde_core.exe"

    if (-not (Test-Path $exe)) {
        Write-Error "PDE core not found at: $exe"
        return
    }

    & $exe @args
}

# 2. Backwards compatibility: `open <stack>`
# Maps to `pde college open <stack>`
function open {
    # If called with no args, just run `pde` to show help
    if ($args.Count -eq 0) {
        pde
        return
    }

    # Pass all arguments to the college open command
    pde college open @args
}