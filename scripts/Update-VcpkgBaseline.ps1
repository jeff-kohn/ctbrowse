# Update-VcpkgBaseline.ps1
# Clones the latest vcpkg repo and runs x-update-baseline vcpkg.json files in the repository.

param(
    [string]$VcpkgRoot = $env:VCPKG_INSTALLATION_ROOT
)

$ErrorActionPreference = 'Stop'

if (-not $VcpkgRoot) {
    throw "-VcpkgRoot not specified and VCPKG_INSTALLATION_ROOT environment variable is not set."
}

$RepoDir   = Split-Path -Parent "$PSScriptRoot"
$VcpkgJson = Join-Path  -Path "$RepoDir"   -ChildPath "vcpkg.json"
$VcpkgExe  = Join-Path  -Path "$VcpkgRoot" -ChildPath "vcpkg.exe"


if (Test-Path (Join-Path $VcpkgRoot ".git")) 
{
   Write-Host "`nUpdating existing vcpkg repo at $VcpkgRoot..." -ForegroundColor Cyan
   try
   {
      Push-Location $VcpkgRoot
      git pull
      if ($LASTEXITCODE -ne 0) { throw "Failed to pull vcpkg repository." }
   }
   finally{ Pop-Location }
}
else 
{
   if (Test-Path $VcpkgRoot) 
   {
      Write-Host "`n"
      Write-Warning "`"$VcpkgRoot`" exists but is not a git repo."
      $reply = Read-Host -Prompt "Delete contents of folder (y/n)?"
      if ($reply -like 'y' -or $reply -like 'Y')
      {
         Write-Output "`nDeleting files..."
         Remove-Item "$VcpkgRoot" -Recurse -Force
         Write-Output "...done.`n"
      }
      else { throw "Couldn't update vcpkg repo, aborting." }
   }

   Write-Host "`nCloning vcpkg to `"$VcpkgRoot`"..." -ForegroundColor Cyan
   git clone https://github.com/microsoft/vcpkg.git "$VcpkgRoot"
   if ($LASTEXITCODE -ne 0) { throw "Failed to clone vcpkg repository." }
}

try
{
   Write-Host "`nBootstrapping vcpkg..." -ForegroundColor Cyan
   Push-Location $VcpkgRoot
   & .\bootstrap-vcpkg.bat
   if ($LASTEXITCODE -ne 0) { throw "Failed to bootstrap vcpkg." }
}
finally{ Pop-Location }

try 
{
   Push-Location $RepoDir
   if (-not (Test-Path $VcpkgJson)) { throw "`"$VcpkgJson`" not found!" }

   Write-Host "`n`nUpdating $VcpkgJson with new baseline commit-ID.."   
   & $VcpkgExe x-update-baseline
   if ($LASTEXITCODE -ne 0) { throw "x-update-baseline failed for `"$VcpkgJson`"" }
}
finally { Pop-Location }
   
Write-Host "`nDone. Review changes with 'git diff'." -ForegroundColor Cyan
