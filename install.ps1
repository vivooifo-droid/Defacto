# Defacto Compiler PowerShell Installer
# Version 0.30

param(
    [string]$InstallPath = "C:\Program Files\Defacto",
    [switch]$NoPrompt
)

$ErrorActionPreference = "Stop"

# Colors
function Write-Success {
    param([string]$Message)
    Write-Host $Message -ForegroundColor Green
}

function Write-Warning-Custom {
    param([string]$Message)
    Write-Host $Message -ForegroundColor Yellow
}

function Write-Error-Custom {
    param([string]$Message)
    Write-Host $Message -ForegroundColor Red
}

# Header
Write-Host "========================================"
Write-Host "  Defacto Compiler Installer v0.30"
Write-Host "========================================"
Write-Host ""

# Check if running as administrator
if (!([Security.Principal.WindowsPrincipal][Security.Principal.WindowsIdentity]::GetCurrent()).IsInRole([Security.Principal.WindowsBuiltInRole] "Administrator")) {
    Write-Warning-Custom "This script should be run as Administrator!"
    Write-Host "Right-click this file and select 'Run with PowerShell'"
    if (!$NoPrompt) {
        $continue = Read-Host "Continue anyway? (y/N)"
        if ($continue -ne 'y' -and $continue -ne 'Y') {
            exit 1
        }
    }
}

# Check dependencies
Write-Host "Checking dependencies..."

$missingDeps = @()

# Check for NASM
try {
    $nasm = Get-Command nasm -ErrorAction SilentlyContinue
    if ($nasm) {
        Write-Success "  NASM found: $($nasm.Source)"
    } else {
        $missingDeps += "NASM"
    }
} catch {
    $missingDeps += "NASM"
}

# Check for g++ or cl (Visual Studio)
$hasCompiler = $false
try {
    $gpp = Get-Command g++ -ErrorAction SilentlyContinue
    if ($gpp) {
        Write-Success "  G++ found: $($gpp.Source)"
        $hasCompiler = $true
    }
} catch {}

if (!$hasCompiler) {
    try {
        $cl = Get-Command cl -ErrorAction SilentlyContinue
        if ($cl) {
            Write-Success "  Visual Studio C++ found"
            $hasCompiler = $true
        }
    } catch {}
}

if (!$hasCompiler) {
    $missingDeps += "C++ Compiler (g++ or Visual Studio)"
}

if ($missingDeps.Count -gt 0) {
    Write-Warning-Custom "Missing dependencies: $($missingDeps -join ', ')"
    Write-Host ""
    Write-Host "Install NASM:"
    Write-Host "  Option 1 - Chocolatey: choco install nasm"
    Write-Host "  Option 2 - Download: https://www.nasm.us/"
    Write-Host ""
    Write-Host "Install Compiler:"
    Write-Host "  Option 1 - Visual Studio Community: https://visualstudio.microsoft.com/"
    Write-Host "  Option 2 - MinGW-w64: https://www.mingw-w64.org/"
    Write-Host ""
    if (!$NoPrompt) {
        $continue = Read-Host "Continue anyway? (y/N)"
        if ($continue -ne 'y' -and $continue -ne 'Y') {
            exit 1
        }
    }
} else {
    Write-Success "All dependencies found!"
}

# Build compiler
Write-Host ""
Write-Host "Building compiler..."

$compilerDir = Join-Path $PSScriptRoot "compiler"

if (!(Test-Path $compilerDir)) {
    Write-Error-Custom "Compiler directory not found: $compilerDir"
    exit 1
}

Push-Location $compilerDir

try {
    # Try make first
    if (Get-Command make -ErrorAction SilentlyContinue) {
        if (make) {
            Write-Success "Build successful!"
        } else {
            Write-Error-Custom "Build failed!"
            exit 1
        }
    } else {
        # Fallback to direct compilation
        Write-Host "Make not found, trying direct compilation..."
        $sources = Get-ChildItem -Path "src" -Filter "*.cpp" | Select-Object -ExpandProperty FullName
        $sources += "main.cpp"
        
        if (Get-Command cl -ErrorAction SilentlyContinue) {
            # Visual Studio
            cl /O2 /Fe:defacto.exe $sources
        } elseif (Get-Command g++ -ErrorAction SilentlyContinue) {
            # MinGW
            g++ -O2 -o defacto.exe $sources
        } else {
            Write-Error-Custom "No compiler found!"
            exit 1
        }
        
        Write-Success "Build successful!"
    }
} catch {
    Write-Error-Custom "Build failed: $_"
    exit 1
}

Pop-Location

# Install
Write-Host ""
Write-Host "Installing to $InstallPath..."

try {
    # Create install directory
    if (!(Test-Path $InstallPath)) {
        New-Item -ItemType Directory -Path $InstallPath | Out-Null
    }
    
    # Copy files
    Copy-Item "$compilerDir\defacto.exe" -Destination $InstallPath -Force
    Copy-Item "$PSScriptRoot\README.md" -Destination $InstallPath -Force
    Copy-Item "$PSScriptRoot\hello.de" -Destination $InstallPath -Force
    Copy-Item "$PSScriptRoot\example-driver.de" -Destination $InstallPath -Force
    
    # Create examples directory
    $examplesDir = Join-Path $InstallPath "examples"
    if (!(Test-Path $examplesDir)) {
        New-Item -ItemType Directory -Path $examplesDir | Out-Null
    }
    
    Write-Success "Files copied successfully!"
} catch {
    Write-Error-Custom "Installation failed: $_"
    exit 1
}

# Add to PATH
Write-Host ""
Write-Host "Adding to PATH..."

try {
    $currentPath = [Environment]::GetEnvironmentVariable("Path", "Machine")
    
    if ($currentPath -notlike "*$InstallPath*") {
        $newPath = "$currentPath;$InstallPath"
        [Environment]::SetEnvironmentVariable("Path", $newPath, "Machine")
        Write-Success "Added to system PATH!"
    } else {
        Write-Warning-Custom "Already in PATH"
    }
} catch {
    Write-Warning-Custom "Could not update PATH: $_"
    Write-Host "Please add manually: $InstallPath"
}

# Create shortcuts
Write-Host ""
Write-Host "Creating shortcuts..."

try {
    $startMenuPath = "$env:PROGRAMDATA\Microsoft\Windows\Start Menu\Programs\Defacto"
    
    if (!(Test-Path $startMenuPath)) {
        New-Item -ItemType Directory -Path $startMenuPath | Out-Null
    }
    
    # Create uninstall shortcut
    $WshShell = New-Object -ComObject WScript.Shell
    $shortcut = $WshShell.CreateShortcut("$startMenuPath\Uninstall.lnk")
    $shortcut.TargetPath = "$InstallPath\uninstall.exe"
    $shortcut.Save()
    
    Write-Success "Shortcuts created!"
} catch {
    Write-Warning-Custom "Could not create shortcuts: $_"
}

# Verify
Write-Host ""
Write-Host "Verifying installation..."

$env:Path = [System.Environment]::GetEnvironmentVariable("Path","Machine") + ";" + [System.Environment]::GetEnvironmentVariable("Path","User")

try {
    & "$InstallPath\defacto.exe" -h 2>&1 | Select-Object -First 5
    Write-Host ""
    Write-Success "========================================"
    Write-Success "  Installation Successful!"
    Write-Success "========================================"
    Write-Host ""
    Write-Host "Quick start:"
    Write-Host "  1. Close and reopen PowerShell/Command Prompt"
    Write-Host "  2. Compile your first program:"
    Write-Host "     defacto -terminal hello.de"
    Write-Host "  3. Run it:"
    Write-Host "     .\hello.exe"
    Write-Host ""
    Write-Host "For more information, visit:"
    Write-Host "  https://github.com/vivooifo-droid/Defacto"
    Write-Host ""
} catch {
    Write-Warning-Custom "Verification test failed, but installation may still work."
    Write-Host "Try restarting your terminal and running: defacto -h"
}

if (!$NoPrompt) {
    Write-Host "Press any key to exit..."
    $null = $Host.UI.RawUI.ReadKey("NoEcho,IncludeKeyDown")
}
