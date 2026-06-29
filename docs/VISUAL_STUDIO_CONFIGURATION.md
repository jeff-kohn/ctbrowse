# Visual Studio Configuration for Line Endings and Code Formatting

This document describes the Visual Studio settings required to ensure the IDE respects repository settings defined in `.gitattributes`, `.editorconfig`, and `.clang-format`.

## Automatic Configuration

The repository is now configured with:

1. **`.editorconfig`** - Comprehensive line ending and formatting settings
2. **`.vs/CMakeWorkspaceSettings.json`** - Git line ending and ClangFormat integration
3. **`.vs/.editorconfig`** - Visual Studio-specific overrides

These files should automatically configure Visual Studio correctly. However, you may need to verify or manually enable certain settings.

## Required Visual Studio Settings

### 1. Git Line Endings

**Path**: `Tools → Options → Text Editor → Advanced`

**Setting**: ✓ Enable **"Use Git attributes for line endings"**

- This ensures Visual Studio respects the `.gitattributes` file
- Prevents VS from automatically converting line endings to CRLF
- Allows Git to manage line endings according to repository rules

### 2. ClangFormat Support

**Path**: `Tools → Options → Text Editor → C/C++ → Code Style → Formatting`

**Settings**:
- ✓ Enable **"Enable ClangFormat support"**
- Set **"Use ClangFormat only for files that contain a .clang-format file in a parent directory"**
- ✓ Enable **"Format on paste"** (optional, recommended)
- ✓ Enable **"Format on save"** (optional, recommended)

This ensures Visual Studio uses the `.clang-format` file in the repository root for C++ code formatting.

### 3. EditorConfig Support

**Path**: `Tools → Options → Text Editor → General`

**Setting**: ✓ Enable **"Follow project coding conventions"**

- This is enabled by default in Visual Studio 2022 and later
- Ensures `.editorconfig` settings take precedence over VS defaults
- Applies line ending, indentation, and other formatting rules from `.editorconfig`

### 4. Line Ending Display (Optional)

**Path**: `Tools → Options → Text Editor → General`

**Setting**: ✓ Enable **"View whitespace"** (Ctrl+R, Ctrl+W)

- Helps visually verify line endings and whitespace
- Shows CR, LF, spaces, and tabs in the editor
- Useful for debugging line ending issues

## Verification

After configuring these settings, you can verify everything is working correctly:

### 1. Check Line Endings

Run the verification script:

```powershell
.\scripts\Verify-LineEndings.ps1
```

This will report any files with incorrect line endings.

### 2. Test New Files

Create a new C++ file in Visual Studio:
- The file should automatically use LF line endings (not CRLF)
- Formatting should follow `.clang-format` rules
- Indentation should be 3 spaces (per `.editorconfig`)

### 3. Check Status Bar

Open any C++ file and check the Visual Studio status bar (bottom right):
- Should show "LF" (not "CRLF")
- You can click on the line ending indicator to change it if needed

## Troubleshooting

### Visual Studio still uses CRLF

1. Verify `.gitattributes` is in the repository root
2. Re-normalize files: `git rm --cached -r . && git reset --hard HEAD`
3. Close and reopen Visual Studio
4. Check Tools → Options → Text Editor → Advanced → "Use Git attributes for line endings" is enabled

### ClangFormat not working

1. Verify `.clang-format` exists in repository root
2. Check Tools → Options → C/C++ → Code Style → Formatting → "Enable ClangFormat support" is enabled
3. Try manually formatting: Right-click → Format Document (Ctrl+K, Ctrl+D)

### EditorConfig not applying

1. Verify `.editorconfig` has `root = true` at the top
2. Check Tools → Options → Text Editor → General → "Follow project coding conventions" is enabled
3. Close and reopen the solution

## Git Configuration

To ensure Git itself handles line endings correctly, set these global Git configs:

```powershell
git config --global core.autocrlf false
git config --global core.eol lf
```

This prevents Git from automatically converting line endings and respects `.gitattributes` explicitly.

## Summary

With the repository configured and Visual Studio settings enabled:

- ✓ All text files use LF line endings (except .bat/.cmd which use CRLF)
- ✓ Visual Studio respects `.gitattributes` and doesn't convert to CRLF
- ✓ C++ files are formatted according to `.clang-format`
- ✓ All formatting follows `.editorconfig` rules
- ✓ Consistent behavior across team members and CI/CD
