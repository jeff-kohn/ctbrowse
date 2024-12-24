# This script will just delete the contents of the build directory. This is sometimes needed
# to get cmake to regenerate the cache so that configure step can run successfully (often occurs
# when editing CMakeLists.txt or CMakePresets.json) 

param
(
   [switch] $Force
)

$ErrorActionPreference = 'Stop'

$RepoDir = Split-Path $PSScriptRoot
$BuildDir = Join-Path $RepoDir builds

Write-Output "`nAbout to delete the contents of $BuildDir."

if (!$Force)
{
   $reply = Read-Host -Prompt "Are you sure you wish to continue? (y/n) "
   if ($reply -like 'y' -or $reply -like 'Y')
   {
      $Force =$true;
   }
}

if ($Force)
{
    Write-Output "Deleting files..."
    Remove-Item $BuildDir/* -Recurse -Force
    Write-Output "...done."
    Write-Output "`nYou will need to run cmake configure before you can build.`n"
}
