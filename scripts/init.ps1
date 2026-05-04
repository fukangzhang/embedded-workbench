$ErrorActionPreference = "Stop"

Write-Host "[init] Checking workspace..."

if (-not (Get-Command git -ErrorAction SilentlyContinue)) {
    throw "git is not installed or not in PATH."
}

if (-not (Get-Command cmake -ErrorAction SilentlyContinue)) {
    throw "cmake is not installed or not in PATH."
}

Write-Host "[init] git version:"
git --version

Write-Host "[init] cmake version:"
cmake --version

Write-Host "[init] Current branch status:"
git status --short --branch

Write-Host "[init] Workspace is ready for bootstrap work."
