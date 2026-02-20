#!/bin/bash
# Build Defacto Windows Installer from macOS/Linux
# Creates defacto-installer.exe for Windows users

set -e

echo "================================"
echo "Building Windows Installer .exe"
echo "================================"
echo ""

# Check dependencies
echo "📋 Checking dependencies..."

if ! command -v python3 &> /dev/null; then
    echo "❌ python3 not found"
    exit 1
fi

if ! command -v pip3 &> /dev/null; then
    echo "❌ pip3 not found"
    exit 1
fi

echo "✅ Python installed"
echo ""

# Install PyInstaller
echo "📦 Installing PyInstaller..."
pip3 install -q pyinstaller

echo "✅ PyInstaller installed"
echo ""

# Build
echo "🔨 Building Windows installer.exe..."
echo ""

python3 -m PyInstaller \
    --onefile \
    --windowed \
    --name "defacto-installer" \
    --distpath "./dist" \
    --workpath "./build" \
    defacto-installer-win.py

echo ""
echo "================================"
echo "✅ Build Complete!"
echo "================================"
echo ""
echo "📁 Output: dist/defacto-installer.exe"
echo ""
echo "Next steps:"
echo "1. Upload to GitHub Releases"
echo "2. Share the download link"
echo "3. Windows users download and run!"
echo ""
