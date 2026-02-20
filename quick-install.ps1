# Defacto Compiler - One-line Installer for Windows
# Download and run with:
#   powershell -Command "iwr https://raw.githubusercontent.com/artemposehonov/defacto/main/quick-install.ps1 -UseBasicParsing | iex"
#
# Or locally:
#   powershell -ExecutionPolicy Bypass -File quick-install.ps1

Write-Host "`n" + ("="*50) -ForegroundColor Cyan
Write-Host "Defacto Compiler - Quick Installer" -ForegroundColor Cyan
Write-Host ("="*50) + "`n" -ForegroundColor Cyan

# Check admin
if (-not ([Security.Principal.WindowsPrincipal][Security.Principal.WindowsIdentity]::GetCurrent()).IsInRole([Security.Principal.WindowsBuiltInRole]::Administrator)) {
    Write-Host "❌ This requires administrator privileges!" -ForegroundColor Red
    Write-Host "Please run: powershell -ExecutionPolicy Bypass -File quick-install.ps1`n" -ForegroundColor Yellow
    exit 1
}

Write-Host "🖥️  Detected: Windows`n" -ForegroundColor Yellow

# Check dependencies
Write-Host "📋 Checking dependencies...`n" -ForegroundColor Yellow

$deps = @("g++", "make")
$missing = @()

foreach ($dep in $deps) {
    if (Get-Command $dep -ErrorAction SilentlyContinue) {
        Write-Host "  ✅ $dep" -ForegroundColor Green
    }
    else {
        Write-Host "  ❌ $dep - NOT FOUND" -ForegroundColor Red
        $missing += $dep
    }
}

if ($missing.Count -gt 0) {
    Write-Host "`n❌ Missing: $($missing -join ', ')" -ForegroundColor Red
    Write-Host "Install with: choco install $($missing -join ' ')`n" -ForegroundColor Yellow
    exit 1
}

Write-Host "`n✅ All dependencies OK`n" -ForegroundColor Green

# Get repository
$repo_dir = Split-Path -Parent $PSScriptRoot

if (-not (Test-Path "$repo_dir\compiler\main.cpp")) {
    Write-Host "📥 Downloading Defacto from GitHub..." -ForegroundColor Yellow
    $temp_dir = [System.IO.Path]::GetTempPath() + "defacto-" + [System.Guid]::NewGuid()
    New-Item -ItemType Directory -Path $temp_dir -Force | Out-Null
    
    try {
        & git clone --depth 1 https://github.com/vivooifo-droid/Defacto.git $temp_dir
        $repo_dir = $temp_dir
    }
    catch {
        Write-Host "❌ Failed to clone repository" -ForegroundColor Red
        exit 1
    }
}

# Build
Write-Host "📦 Building compiler..." -ForegroundColor Yellow
$compiler_dir = "$repo_dir\compiler"

Push-Location $compiler_dir
try {
    & make clean 2>$null
    & make windows 2>&1 | Out-Null
    if (-not (Test-Path "defacto.exe")) {
        throw "Build failed"
    }
}
catch {
    Write-Host "❌ Build failed" -ForegroundColor Red
    exit 1
}
Pop-Location

Write-Host "✅ Built: defacto.exe`n" -ForegroundColor Green

# Install
$install_path = "$env:ProgramFiles\Defacto"
Write-Host "📁 Installing to $install_path..." -ForegroundColor Yellow

if (-not (Test-Path $install_path)) {
    New-Item -ItemType Directory -Path $install_path -Force | Out-Null
}

Copy-Item "$compiler_dir\defacto.exe" "$install_path\" -Force

# Add to PATH
$env_path = [Environment]::GetEnvironmentVariable("Path", "User")
if (-not $env_path.Contains($install_path)) {
    [Environment]::SetEnvironmentVariable("Path", "$env_path;$install_path", "User")
    Write-Host "✅ Added to PATH (restart terminal)" -ForegroundColor Green
}

# Refresh current session
$env:Path = [System.Environment]::GetEnvironmentVariable("Path","Machine") + ";" + [System.Environment]::GetEnvironmentVariable("Path","User")

Write-Host "`n" + ("="*50) -ForegroundColor Cyan
Write-Host "✅ Installation Complete!" -ForegroundColor Green
Write-Host ("="*50) -ForegroundColor Cyan

Write-Host "`nUsage:" -ForegroundColor Yellow
Write-Host "  defacto -h                    # Help"
Write-Host "  defacto -terminal hello.de    # Compile"
Write-Host "`nDon't forget NASM:" -ForegroundColor Cyan
Write-Host "  choco install nasm`n" -ForegroundColor Yellow
