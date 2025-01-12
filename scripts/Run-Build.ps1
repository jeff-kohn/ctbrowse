<#
  .SYNOPSIS
  Build project using the specified CMake Preset

  .DESCRIPTION
   This script will build the specified preset/configuration from the command line.
   Cmake should be in your path, if not you should run this script from a VS Developer
   Command Prompt.

  .PARAMETER Preset
  Specifies the CMake build preset to use. Defaults to "win-msvc".

  .PARAMETER Config
  Optionally specify the build configuration for multi-config presets that don't specify
  one. No default supplied since some presets will already specify a build configuration.

  .PARAMETER Rebuild
  If this switch is supplied, the --clean-first build will be passed to cmake for a clean build.
  Clean build DOES not delete CMake cache or re-run configure, it just deletes existing build binaries.

  .PARAMETER RunTests
  This switch causes cmake to run unit tests

  .EXAMPLE
  PS> scripts/Build-Preset.ps1 -Preset="win-msvc" -Config="Release"

  .EXAMPLE
  PS> scripts/Build-Preset.ps1 win-debug 

  .EXAMPLE
  PS> scripts/Build-Preset.ps1 -Config="Debug" -Rebuild -Tests

#>


param
(
   [string] $Preset = "dev-vscode",
   [string] $Config = "",
   [string] $Target = "all",
   [switch] $Rebuild,
   [switch] $RunTests
)

$ErrorActionPreference = 'Stop'

$RepoDir = Split-Path $PSScriptRoot
$saved_location = Get-Location
Set-Location $RepoDir

try
{
   Write-Host "Building project for $Preset using repo dir $RepoDir..." -ForegroundColor Cyan

   if ( !(Test-Path "$RepoDir/build/CMakeCache.txt") )
   {
      Write-Host "`r`nBuild directory not found. Run configure before building.`r`n"
      exit (-1)
   }

   if ($Config)
   {
      $ConfigArg = "--config=$Config"
   }

   if ( $Rebuild )
   {
      cmake --build --preset=$Preset $ConfigArg --target=$Target --clean-first
   }
   else
   {
      cmake --build --preset=$Preset  $ConfigArg --target=$Target
   }

   if ( $RunTests )
   {
      if ($Config)
      {
         $ConfigArg = "-C $Config"
      }
      else{
         $ConfigArg = ""
      }
      Write-Host "`r`n`tctest --preset $Preset $ConfigArg --output-on-failure --output-junit `"$Preset.test_results.xml`"`r`n"
      ctest --preset $Preset $Config --output-on-failure --output-junit "$Preset.test_results.xml"
   }
}
finally
{
   Set-Location $saved_location
}
