<#
.SYNOPSIS
    Runs clang-format on C++ source files in the repository.

.DESCRIPTION
    Enumerates *.cpp and *.h files in the specified folders and runs clang-format
    on each file. Supports dry-run mode (default) for reporting formatting issues
    and fix mode for in-place corrections.

.PARAMETER ClangFormatPath
    Path to the clang-format executable. Defaults to "$env:ProgramFiles\LLVM\bin\clang-format".

.PARAMETER Folders
    List of folders to recurse for source files. Defaults to @("include", "lib").

.PARAMETER DryRun
    Run clang-format with --dry-run to report formatting issues without modifying files.
    This is the default behavior.

.PARAMETER Fix
    Run clang-format with -i to fix formatting issues in-place.
    Mutually exclusive with -DryRun.

.EXAMPLE
    .\Run-ClangFormat.ps1
    Runs clang-format in dry-run mode on all source files in include/ and lib/.

.EXAMPLE
    .\Run-ClangFormat.ps1 -Fix
    Fixes formatting in-place for all source files in include/ and lib/.

.EXAMPLE
    .\Run-ClangFormat.ps1 -Folders "include","lib","examples" -DryRun
    Runs clang-format in dry-run mode on source files in include/, lib/, and examples/.
#>
[CmdletBinding()]
param(
    [string]$ClangFormatPath = "$env:ProgramFiles\LLVM\bin\clang-format.exe",
    [string[]]$Folders = @("include", "ctBrowse_lib", "ctBrowse_app"),
    [switch]$DryRun,
    [switch]$Fix
)

$ErrorActionPreference = 'Stop'

# Validate mutual exclusivity
if ($DryRun -and $Fix) {
    throw "-DryRun and -Fix are mutually exclusive. Specify only one."
}

# Resolve repo root (parent of scripts/)
$repoRoot = Split-Path -Parent $PSScriptRoot

# Validate clang-format exists
if (-not (Test-Path $ClangFormatPath)) {
    throw "clang-format not found at '$ClangFormatPath'. Specify -ClangFormatPath or install LLVM."
}

# Build clang-format arguments
$formatArgs = @()
if ($Fix) {
    $formatArgs += '-i'
} else {
    # Default to --dry-run
    $formatArgs += '--dry-run'
}

# Collect source files from specified folders
$sourceFiles = @()
foreach ($folder in $Folders) {
    $folderPath = Join-Path $repoRoot $folder
    if (-not (Test-Path $folderPath)) {
        Write-Warning "Folder not found: $folderPath — skipping."
        continue
    }
    $files = Get-ChildItem -Path $folderPath -Recurse -Include *.cpp, *.h
    $sourceFiles += $files
}

if ($sourceFiles.Count -eq 0) {
    Write-Host "No source files found in specified folders." -ForegroundColor Yellow
    exit 0
}

Write-Host "Running clang-format on $($sourceFiles.Count) file(s)..." -ForegroundColor Cyan

$failCount = 0
foreach ($file in $sourceFiles) {
    $file_args = $formatArgs + $file.FullName
    & $ClangFormatPath @file_args
    if ($LASTEXITCODE -ne 0) {
        $failCount++
    }
}

if ($failCount -gt 0) {
    Write-Host "$failCount file(s) have formatting issues." -ForegroundColor Red
    exit 1
} else {
    Write-Host "All files pass clang-format." -ForegroundColor Green
    exit 0
}
