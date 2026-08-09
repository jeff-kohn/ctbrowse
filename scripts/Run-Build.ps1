<#
.SYNOPSIS
   Runs cmake configure and build commands for the specified target.

.DESCRIPTION
    Builds the ctBrowse project for Windows using the specific CMake preset. This script assumes that the build preset's name contains
    the configure preset name followed by "-release" or "-whatever". If configure preset can't be extracted from the build preset and you
    don't specify -SkipConfigure, you'll probably get a cmake error about invalid configure preset name.


.PARAMETER BuildPreset
    Name of the build preset to use (case-sensitive). Defaults to 'win-msvc-x64-release'

.PARAMETER Target
    Specifies the build target (case-sensitive). Defaults to "ALL_BUILD".
    Can also be "install" "code_analysis" or a specific project target. Presets using
    ninja generator will need to specify "all" instead of "ALL_BUILD" for the default target.

.PARAMETER SkipConfigure
    If set, skips the cmake configure step and runs only the build command.

.PARAMETER Rebuild
    If set, passes --clean-first to cmake for a clean build.

.EXAMPLE
    .\build.ps1
    Builds with default preset (win-msvc-x64-release) and ALL_BUILD target.

.EXAMPLE
    .\build.ps1 -BuildPreset 'win-msvc-x86-debug' -Target 'install'
    Builds with x86 debug preset and creates install package/folder.

.EXAMPLE
    .\build.ps1 -SkipConfigure -Target 'code_analysis'
   Runs code analysis without reconfiguring cmake.
#>

[CmdletBinding()]
param(
   [string]$BuildPreset = 'win-msvc-x64-release',

   [string]$Target = 'ALL_BUILD',

   [switch]$SkipConfigure,

   [switch]$Rebuild
)

$ErrorActionPreference = 'Stop'

function Get-ConfigPreset([string] $BuildPresetName)
{
   $lastDash = $BuildPresetName.LastIndexOf('-')
   $ConfigPreset = if ($lastDash -ge 0) {
      $BuildPresetName.Substring(0, $lastDash)
   }
   else {
      $BuildPresetName  # fallback if no '-' exists
   }
   return $ConfigPreset
}


# Main Entry Point
try {
   $repoDir = Split-Path -Parent $PSScriptRoot
   $savedLocation = Get-Location

   Set-Location $repoDir

   Write-Host "Building ctBrowse for $BuildPreset... using repo dir $repoDir"

   if (-not $SkipConfigure)
   {
      $PresetName = Get-ConfigPreset -BuildPresetName $BuildPreset
      $ConfigureArgs = @("--preset=$PresetName", "-Wno-author")

      Write-Host "Running configure..."
      cmake @ConfigureArgs
      if ($LASTEXITCODE -ne 0) { throw "CMake configure failed with exit code $LASTEXITCODE" }
   }

   $BuildArgs = @("--build", "--preset=$BuildPreset", "--target=$Target")
   if ($Rebuild)
   {
      Write-Host "Running clean/build for target $Target..."
      $BuildArgs += "--clean-first"
   }
   else {
      Write-Host "Running build for target $Target..."
   }

   cmake @BuildArgs
   if ($LASTEXITCODE -ne 0) { throw "CMake build ($BuildPreset) failed with exit code $LASTEXITCODE" }
}
finally {
   Set-Location $savedLocation
}
