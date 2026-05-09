param(
    [string]$OpenOcd = "openocd",
    [string]$ImagePath = "build-fw-real-usart2-command\embedded_firmware.elf",
    [string]$InterfaceConfig = "interface/stlink.cfg",
    [string]$TargetConfig = "target/stm32f4x.cfg",
    [string]$FlashAddress = "0x08000000",
    [switch]$DryRun
)

$ErrorActionPreference = "Stop"

$RepoRoot = (Resolve-Path (Join-Path $PSScriptRoot "..")).Path

if ([System.IO.Path]::IsPathRooted($ImagePath)) {
    $ResolvedImage = Resolve-Path $ImagePath
} else {
    $ResolvedImage = Resolve-Path (Join-Path $RepoRoot $ImagePath)
}

$ImageExtension = [System.IO.Path]::GetExtension($ResolvedImage.Path).ToLowerInvariant()
$OpenOcdImagePath = $ResolvedImage.Path -replace "\\", "/"

if ($OpenOcdImagePath.Contains("}")) {
    throw "Image path contains '}', which cannot be safely wrapped for the OpenOCD Tcl command."
}

$ProgramParts = @("program", "{$OpenOcdImagePath}")

if ($ImageExtension -notin @(".elf", ".hex", ".bin")) {
    Write-Warning "Image extension '$ImageExtension' is not one of .elf, .hex, or .bin; OpenOCD may need extra arguments."
}

$ProgramParts += @("verify", "reset", "exit")

if ($ImageExtension -eq ".bin") {
    $ProgramParts += $FlashAddress
}

$ProgramCommand = $ProgramParts -join " "
$OpenOcdArgs = @(
    "-f", $InterfaceConfig,
    "-f", $TargetConfig,
    "-c", $ProgramCommand
)

Write-Host "[flash] Repository: $RepoRoot"
Write-Host "[flash] Image: $($ResolvedImage.Path)"
Write-Host "[flash] Interface config: $InterfaceConfig"
Write-Host "[flash] Target config: $TargetConfig"
Write-Host "[flash] OpenOCD command: $ProgramCommand"

if ($DryRun) {
    $DisplayArgs = $OpenOcdArgs | ForEach-Object {
        if ($_ -match "\s") {
            '"' + ($_ -replace '"', '\"') + '"'
        } else {
            $_
        }
    }
    Write-Host "[flash] Dry run only; OpenOCD was not executed."
    Write-Host "[flash] Full command:"
    Write-Host "$OpenOcd $($DisplayArgs -join ' ')"
    exit 0
}

if (-not (Get-Command $OpenOcd -ErrorAction SilentlyContinue)) {
    throw "OpenOCD executable '$OpenOcd' was not found. Install OpenOCD or pass -OpenOcd with the executable path."
}

& $OpenOcd @OpenOcdArgs

if ($LASTEXITCODE -ne 0) {
    throw "OpenOCD failed with exit code $LASTEXITCODE."
}
