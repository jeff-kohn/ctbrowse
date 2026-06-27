<#
  .SYNOPSIS
  Use run-clang-tidy to scan the entire project. Requires a succesful configuratin to generate compile_commands.json
#>
param
(
   [string] $BuildDir = "build\ci-static-analysis\",
   [string] $RunClangTidyPath = "$env:ProgramFiles\LLVM\bin\run-clang-tidy",
   [int] $Jobs = 16,
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
      Write-Host "Running clang-tidy with fix enabled for $BuildDir ..." -ForegroundColor Cyan
      $FixArg = "-fix"
   }
   else
   {
      Write-Host "Running clang-tidy for $BuildDir ..." -ForegroundColor Cyan
      $FixArg = ""
   }
   $JobsArg = "-j $Jobs"
   python "$RunClangTidyPath" -source-filter '.*ctBrowse.*\.cpp' -quiet -use-color $JobsArg $FixArg -p $BuildDir
}
finally
{
   Set-Location $saved_location
}
