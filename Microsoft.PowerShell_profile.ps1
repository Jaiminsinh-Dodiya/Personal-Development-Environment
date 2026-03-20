function open {
    param(
        [Parameter(Mandatory, Position = 0)]
        [string]$Category,

        [Parameter(Position = 1)]
        [string]$Tech,

        [Parameter(Position = 2)]
        [string]$Faculty
    )

    $exe = "D:\PDE\bin\pde_core.exe"

    if (-not (Test-Path $exe)) {
        Write-Error "PDE core not found at: $exe"
        return
    }

    # Build args dynamically — avoids passing empty strings to the C++ side
    $argList = @($Category)
    if ($Tech)    { $argList += $Tech }
    if ($Faculty) { $argList += $Faculty }

    & $exe @argList
}