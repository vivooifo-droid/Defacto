#!/bin/bash
# Build Windows installer for Defacto Compiler
# Requires: mingw-w64, nsis (makensis)

set -e

VERSION="0.40"
COMPILER_DIR="compiler"
INSTALLER_NSI="installer.nsi"
OUTPUT="defacto-${VERSION}-installer.exe"

echo "=== Defacto Windows Installer Builder ==="

# Check dependencies
command -v x86_64-w64-mingw32-g++ >/dev/null 2>&1 || { echo "❌ mingw-w64 not found. Install: brew install mingw-w64"; exit 1; }
command -v makensis >/dev/null 2>&1 || { echo "❌ NSIS not found. Install: brew install nsis"; exit 1; }
command -v x86_64-w64-mingw32-strip >/dev/null 2>&1 || { echo "❌ mingw-strip not found"; exit 1; }

# Step 1: Build Windows compiler binary
echo "📦 Building Windows compiler..."
cd "$COMPILER_DIR"

make clean 2>/dev/null || true
make windows

if [ ! -f "defacto.exe" ]; then
    echo "❌ Failed to build defacto.exe"
    exit 1
fi

# Strip binary
x86_64-w64-mingw32-strip defacto.exe 2>/dev/null || true

echo "✅ Built: defacto.exe ($(du -h defacto.exe | cut -f1))"
cd ..

# Step 2: Prepare installer package
echo "📁 Preparing installer package..."
INSTALLER_PKG="installer_pkg"
rm -rf "$INSTALLER_PKG"
mkdir -p "$INSTALLER_PKG"

cp "$COMPILER_DIR/defacto.exe" "$INSTALLER_PKG/"
cp "hello.de" "$INSTALLER_PKG/" 2>/dev/null || echo "// Example program" > "$INSTALLER_PKG/hello.de"
cp "README.md" "$INSTALLER_PKG/README.txt"

# Step 3: Build NSIS installer using main installer.nsi
echo "🔨 Building NSIS installer..."
cd "$INSTALLER_PKG"

# Copy main installer.nsi to package directory
cp "../installer.nsi" .

makensis -V2 installer.nsi

if [ ! -f "defacto-0.40-installer.exe" ]; then
    echo "❌ Failed to build installer"
    exit 1
fi

mv "defacto-0.40-installer.exe" "../$OUTPUT"
cd ..

# Cleanup
rm -rf "$INSTALLER_PKG"

echo ""
echo "✅ Success! Created: $OUTPUT"
echo "📦 Size: $(du -h "$OUTPUT" | cut -f1)"
echo ""
echo "To test: wine $OUTPUT"
