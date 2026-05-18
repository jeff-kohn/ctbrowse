<#
  .SYNOPSIS
  Use run-clang-tidy to scan the entire project. Requires a succesful configuratin to generate compile_commands.json
#>
param
(
   [string] $ConfigPreset = "run-clang-tidy",
   [string] $RunClangTidyPath = "$env:ProgramFiles\LLVM\bin\run-clang-tidy",
   [switch] $Fix
)

$ErrorActionPreference = 'Stop'

$RepoDir = Split-Path $PSScriptRoot
$saved_location = Get-Location
Write-Host "current directory: $saved_location"
Write-Host "changing to $RepoDir"
Set-Location $RepoDir

try
{
   if ($Fix)
   {
      Write-Host "Running clang-tidy with fix enabled for $ConfigPreset..." -ForegroundColor Cyan
      $FixArg = "-fix style file"
   }
   else
   {
      Write-Host "Running clang-tidy for $ConfigPreset..." -ForegroundColor Cyan
      $FixArg = ""
   }
   python "$RunClangTidyPath" -source-filter .*\.cpp -quiet $FixArg -p build\$ConfigPreset\
}
finally
{
   Set-Location $saved_location
}
