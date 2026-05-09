param(
    [string]$BuildDir = "build-fw-scheduler-real-usart2-io",
    [string]$CMake = "C:\Users\fukan\.codex\venvs\embedded-workbench-toolchain\Scripts\cmake.exe",
    [string]$Ninja = "C:\Users\fukan\.codex\venvs\embedded-workbench-toolchain\Scripts\ninja.exe",
    [string]$Zig = "C:\Users\fukan\.codex\venvs\embedded-workbench-toolchain\Lib\site-packages\ziglang\zig.exe",
    [string]$ZigAr = "C:\Users\fukan\.codex\venvs\embedded-workbench-toolchain\Scripts\zig-ar.cmd",
    [string]$ZigRanlib = "C:\Users\fukan\.codex\venvs\embedded-workbench-toolchain\Scripts\zig-ranlib.cmd",
    [string]$FreeRtosKernelPath = "C:\Users\fukan\.codex\deps\FreeRTOS-Kernel",
    [switch]$DryRun
)

$ErrorActionPreference = "Stop"

function Resolve-ToolPath {
    param(
        [string]$Value,
        [string]$Name
    )

    if ([System.IO.Path]::IsPathRooted($Value)) {
        return (Resolve-Path $Value).Path
    }

    $Command = Get-Command $Value -ErrorAction SilentlyContinue
    if ($null -eq $Command) {
        throw "$Name '$Value' was not found. Pass -$Name with the executable path."
    }

    return $Command.Source
}

function Format-CommandLine {
    param(
        [string]$Command,
        [string[]]$Arguments
    )

    $DisplayArgs = $Arguments | ForEach-Object {
        if ($_ -match "\s") {
            '"' + ($_ -replace '"', '\"') + '"'
        } else {
            $_
        }
    }

    return "$Command $($DisplayArgs -join ' ')"
}

$RepoRoot = (Resolve-Path (Join-Path $PSScriptRoot "..")).Path
$BuildPath = if ([System.IO.Path]::IsPathRooted($BuildDir)) {
    [System.IO.Path]::GetFullPath($BuildDir)
} else {
    [System.IO.Path]::GetFullPath((Join-Path $RepoRoot $BuildDir))
}

$CMakePath = Resolve-ToolPath $CMake "CMake"
$NinjaPath = Resolve-ToolPath $Ninja "Ninja"
$ZigPath = Resolve-ToolPath $Zig "Zig"
$ZigArPath = Resolve-ToolPath $ZigAr "ZigAr"
$ZigRanlibPath = Resolve-ToolPath $ZigRanlib "ZigRanlib"
$FreeRtosPath = (Resolve-Path $FreeRtosKernelPath).Path
$ToolchainPath = (Resolve-Path (Join-Path $RepoRoot "cmake\toolchains\zig-arm-none-eabi.cmake")).Path

$ConfigureArgs = @(
    "-S", $RepoRoot,
    "-B", $BuildPath,
    "-G", "Ninja",
    "-DCMAKE_MAKE_PROGRAM=$NinjaPath",
    "-DZIG_EXE=$ZigPath",
    "-DZIG_AR=$ZigArPath",
    "-DZIG_RANLIB=$ZigRanlibPath",
    "-DCMAKE_TOOLCHAIN_FILE=$ToolchainPath",
    "-DEW_BUILD_HOST_TOOLS=OFF",
    "-DEW_BUILD_TESTS=OFF",
    "-DEW_BUILD_FIRMWARE=ON",
    "-DEW_USE_FREERTOS=ON",
    "-DEW_FREERTOS_START_SCHEDULER=ON",
    "-DEW_FIRMWARE_USE_REAL_STM32_USART2_FREERTOS_COMMAND_READER=ON",
    "-DEW_FIRMWARE_USE_REAL_STM32_USART2_FREERTOS_RESPONSE_WRITER=ON",
    "-DFREERTOS_KERNEL_PATH=$FreeRtosPath"
)

$BuildArgs = @("--build", $BuildPath)

Write-Host "[build] Repository: $RepoRoot"
Write-Host "[build] Build directory: $BuildPath"
Write-Host "[build] FreeRTOS-Kernel: $FreeRtosPath"
Write-Host "[build] Firmware image: $(Join-Path $BuildPath 'embedded_firmware.elf')"

if ($DryRun) {
    Write-Host "[build] Dry run only; CMake was not executed."
    Write-Host "[build] Configure command:"
    Write-Host (Format-CommandLine $CMakePath $ConfigureArgs)
    Write-Host "[build] Build command:"
    Write-Host (Format-CommandLine $CMakePath $BuildArgs)
    exit 0
}

& $CMakePath @ConfigureArgs
if ($LASTEXITCODE -ne 0) {
    throw "CMake configure failed with exit code $LASTEXITCODE."
}

& $CMakePath @BuildArgs
if ($LASTEXITCODE -ne 0) {
    throw "CMake build failed with exit code $LASTEXITCODE."
}
