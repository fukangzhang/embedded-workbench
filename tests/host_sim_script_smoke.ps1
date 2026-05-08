$ErrorActionPreference = "Stop"

$hostSim = (Resolve-Path -LiteralPath $args[0]).Path
$command = "echo STATUS? | ""$hostSim"" --script"
$output = (cmd /c $command) -join "`n"

if ($LASTEXITCODE -ne 0) {
    Write-Error "host_sim exited with $LASTEXITCODE"
}

if (-not $output.Contains("OK result=ok")) {
    Write-Error "Missing command result response. Output was: $output"
}

if (-not $output.Contains("STATUS state=normal")) {
    Write-Error "Missing status response"
}

if (-not $output.Contains("indicator=off")) {
    Write-Error "Missing alarm output summary"
}
