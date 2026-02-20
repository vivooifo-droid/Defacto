# Defacto Compiler - Install Script for Windows (PowerShell)
# Usage: powershell -ExecutionPolicy Bypass -File install.ps1

$ErrorActionPreference = "Stop"

Write-Host "================================" -ForegroundColor Cyan
Write-Host "Defacto Compiler Installer" -ForegroundColor Cyan
Write-Host "================================" -ForegroundColor Cyan
Write-Host ""

# Check admin privileges
if (-not ([Security.Principal.WindowsPrincipal][Security.Principal.WindowsIdentity]::GetCurrent()).IsInRole([Security.Principal.WindowsBuiltInRole]::Administrator)) {
    Write-Host "❌ This script requires administrator privileges!" -ForegroundColor Red
    Write-Host "Please run PowerShell as Administrator" -ForegroundColor Red
    exit 1
}

# Check dependencies
Write-Host "📋 Checking dependencies..." -ForegroundColor Yellow

$deps_ok = $true

if (-not (Get-Command g++ -ErrorAction SilentlyContinue)) {
    Write-Host "❌ g++ not found. Install MinGW-w64 or Visual Studio Community" -ForegroundColor Red
    $deps_ok = $false
}

if (-not (Get-Command make -ErrorAction SilentlyContinue)) {
    Write-Host "❌ make not found. Install with: choco install make" -ForegroundColor Red
    $deps_ok = $false
}

if (-not (Get-Command nasm -ErrorAction SilentlyContinue)) {
    Write-Host "⚠️  nasm not found. Install with: choco install nasm" -ForegroundColor Yellow
    $choice = Read-Host "Continue anyway? (y/n)"
    if ($choice -ne "y") {
        exit 1
    }
}

if (-not $deps_ok) {
    exit 1
}

Write-Host "✅ Dependencies OK" -ForegroundColor Green
Write-Host ""

# Build compiler
Write-Host "📦 Building compiler for Windows..." -ForegroundColor Yellow

$compiler_dir = Join-Path $PSScriptRoot "compiler"
Push-Location $compiler_dir
try {
    & make clean 2>$null
    & make windows 2>&1 | Out-Null
    if (-not (Test-Path "defacto.exe")) {
        throw "Failed to build defacto.exe"
    }
}
finally {
    Pop-Location
}

Write-Host "✅ Compiler built: defacto.exe" -ForegroundColor Green
Write-Host ""

# Install to Program Files
$install_dir = "$env:ProgramFiles\Defacto"
Write-Host "📁 Installing to $install_dir..." -ForegroundColor Yellow

if (-not (Test-Path $install_dir)) {
    New-Item -ItemType Directory -Path $install_dir -Force | Out-Null
}

Copy-Item (Join-Path $compiler_dir "defacto.exe") $install_dir -Force

# Add to PATH
$env_path = [Environment]::GetEnvironmentVariable("Path", "User")
if (-not $env_path.Contains($install_dir)) {
    [Environment]::SetEnvironmentVariable("Path", "$env_path;$install_dir", "User")
    Write-Host "✅ Added to PATH" -ForegroundColor Green
}
else {
    Write-Host "✅ Already in PATH" -ForegroundColor Green
}

# Refresh PATH for current session
$env:Path = [System.Environment]::GetEnvironmentVariable("Path","Machine") + ";" + [System.Environment]::GetEnvironmentVariable("Path","User")

# Create Start Menu shortcuts
$start_menu = "$env:ProgramData\Microsoft\Windows\Start Menu\Programs\Defacto"
if (-not (Test-Path $start_menu)) {
    New-Item -ItemType Directory -Path $start_menu -Force | Out-Null
}

$shortcut = (New-Object -ComObject WScript.Shell).CreateShortcut("$start_menu\Defacto Compiler.lnk")
$shortcut.TargetPath = "$install_dir\defacto.exe"
$shortcut.Save()

Write-Host ""
Write-Host "================================" -ForegroundColor Cyan
Write-Host "✅ Installation complete!" -ForegroundColor Green
Write-Host "================================" -ForegroundColor Cyan
Write-Host ""
Write-Host "Usage:" -ForegroundColor Yellow
Write-Host "  defacto -terminal hello.de"
Write-Host "  defacto -kernel -o kernel.bin os.de"
Write-Host "  defacto -h"
Write-Host ""

# Verify installation
if (Get-Command defacto -ErrorAction SilentlyContinue) {
    Write-Host "✅ defacto is ready to use!" -ForegroundColor Green
    & defacto -h | Select-Object -First 5
}
else {
    Write-Host "⚠️  Please close and reopen PowerShell to use defacto" -ForegroundColor Yellow
}

Write-Host ""
Write-Host "Don't forget to install NASM if not already installed:" -ForegroundColor Cyan
Write-Host "  choco install nasm" -ForegroundColor Cyan
Write-Host "  or" -ForegroundColor Cyan
Write-Host "  https://www.nasm.us/" -ForegroundColor Cyan
