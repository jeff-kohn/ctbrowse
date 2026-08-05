#!/usr/bin/env pwsh
<#
.SYNOPSIS
	Verify line endings across the repository match .gitattributes specifications.

.DESCRIPTION
	This script checks all tracked files in the repository to ensure they have
	the correct line endings as specified in .gitattributes:
	- C++, CMake, config files should use LF
	- .bat/.cmd files should use CRLF

	Reports any files with incorrect line endings.

.EXAMPLE
	.\Verify-LineEndings.ps1
#>

[CmdletBinding()]
param()

Write-Host "`n=== Line Ending Verification ===" -ForegroundColor Cyan
Write-Host ""

# Get all files with their line ending information
$allFiles = git ls-files --eol | ForEach-Object {
	if ($_ -match '^i/(\S+)\s+w/(\S+)\s+attr/(.+?)\s+(.+)$') {
		[PSCustomObject]@{
			IndexEOL = $Matches[1]
			WorkingEOL = $Matches[2]
			Attributes = $Matches[3]
			FilePath = $Matches[4]
		}
	}
}

# Categorize files
$correctLF = @()
$correctCRLF = @()
$incorrectCRLF = @()
$other = @()

foreach ($file in $allFiles) {
	# Files that should have LF
	if ($file.Attributes -match 'eol=lf') {
		if ($file.WorkingEOL -eq 'lf') {
			$correctLF += $file
		}
		elseif ($file.WorkingEOL -eq 'crlf') {
			$incorrectCRLF += $file
		}
		else {
			$other += $file
		}
	}
	# Files that should have CRLF
	elseif ($file.Attributes -match 'eol=crlf') {
		if ($file.WorkingEOL -eq 'crlf') {
			$correctCRLF += $file
		}
		else {
			$incorrectCRLF += $file
		}
	}
	# Files with text=auto or other
	else {
		$other += $file
	}
}

# Display results
Write-Host "✓ Files with correct LF endings: " -NoNewline -ForegroundColor Green
Write-Host $correctLF.Count

if ($correctCRLF.Count -gt 0) {
	Write-Host "✓ Files with correct CRLF endings (.bat/.cmd): " -NoNewline -ForegroundColor Green
	Write-Host $correctCRLF.Count
}

if ($other.Count -gt 0) {
	Write-Host "• Files with other/auto endings: " -NoNewline -ForegroundColor Yellow
	Write-Host $other.Count
}

# Report issues
if ($incorrectCRLF.Count -gt 0) {
	Write-Host ""
	Write-Host "✗ Files with INCORRECT line endings:" -ForegroundColor Red
	Write-Host ""

	foreach ($file in $incorrectCRLF) {
		$expected = if ($file.Attributes -match 'eol=lf') { 'LF' } else { 'CRLF' }
		$actual = $file.WorkingEOL.ToUpper()
		Write-Host "  $($file.FilePath)" -ForegroundColor Red
		Write-Host "    Expected: $expected, Got: $actual" -ForegroundColor Yellow
	}

	Write-Host ""
	Write-Host "To fix these files, run:" -ForegroundColor Yellow
	Write-Host "  git rm --cached -r ." -ForegroundColor Cyan
	Write-Host "  git reset --hard HEAD" -ForegroundColor Cyan
	Write-Host ""

	exit 1
}
else {
	Write-Host ""
	Write-Host "✓ All files have correct line endings!" -ForegroundColor Green
	Write-Host ""

	# Show summary by file type
	Write-Host "Summary by file type:" -ForegroundColor Cyan
	$correctLF | Group-Object { [System.IO.Path]::GetExtension($_.FilePath) } | 
		Sort-Object Count -Descending | 
		Select-Object @{N='Extension';E={if($_.Name -eq '') {'(no extension)'} else {$_.Name}}}, Count |
		Format-Table -AutoSize

	exit 0
}
