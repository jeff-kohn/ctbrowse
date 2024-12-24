
param
(
   [string] $Config = "Debug"
)

$ErrorActionPreference = 'Stop'

$RepoDir = Split-Path $PSScriptRoot
$TestsDir = Join-Path $RepoDir "builds/tests"
$ExeDir = Join-Path $TestsDir $Config
$TestExe = Join-Path $ExeDir "tests.exe"

$saved_location = Get-Location
Set-Location $TestsDir

try
{
   Write-Host "running OpenCppCoverage with the folllowing values: RepoDir=$RepoDir, ExeDir=$ExeDir, TestsDir=$TestsDir, TestExe=$TestExe"
   
   & $env:ProgramFiles\OpenCppCoverage\opencppcoverage.exe --verbose `
      --modules $TestExe                              `
      --sources $RepoDir\src                                         `
      --sources $ReporDir\include                                    `
      --excluded_sources $env:ProgramFiles                           `
      --excluded_sources "D:\a\_work\1"                              `
      --excluded_sources $env:VCPKG_INSTALLATION_ROOT                `
      --excluded_sources $BinDir\vcpkg_installed                     `
      --excluded_sources $BinDir\_deps                               `
      --export_type=cobertura:"..\code_coverage.xml"                 `
      --export_type=html:"..\code_coverage_html"                     `
      -- $TestExe
}
finally
{
   Set-Location $saved_location
}
