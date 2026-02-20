#!/bin/bash
# Defacto Compiler - Universal Install Script for macOS/Linux
# Usage: bash install.sh [--prefix /path/to/install]

set -e

INSTALL_PREFIX="${1:---prefix}"
if [ "$INSTALL_PREFIX" = "--prefix" ] && [ -n "$2" ]; then
    INSTALL_PATH="$2"
else
    INSTALL_PATH="/usr/local/bin"
fi

echo "================================"
echo "Defacto Compiler Installer"
echo "================================"
echo ""

# Check dependencies
echo "📋 Checking dependencies..."

if ! command -v g++ &> /dev/null; then
    echo "❌ g++ not found. Install with:"
    echo "  macOS: xcode-select --install"
    echo "  Linux (Ubuntu): sudo apt install build-essential"
    exit 1
fi

if ! command -v make &> /dev/null; then
    echo "❌ make not found. Install with:"
    echo "  macOS: brew install make"
    echo "  Linux (Ubuntu): sudo apt install make"
    exit 1
fi

if ! command -v nasm &> /dev/null; then
    echo "⚠️  nasm not found. Install with:"
    echo "  macOS: brew install nasm"
    echo "  Linux (Ubuntu): sudo apt install nasm"
    echo ""
    read -p "Continue anyway? (y/n) " -n 1 -r
    echo
    if [[ ! $REPLY =~ ^[Yy]$ ]]; then
        exit 1
    fi
fi

echo "✅ Dependencies OK"
echo ""

# Build compiler
echo "📦 Building compiler..."
cd "$(dirname "$0")/compiler"
make clean > /dev/null 2>&1
make > /dev/null 2>&1
cd - > /dev/null

echo "✅ Compiler built"
echo ""

# Install
echo "📁 Installing to $INSTALL_PATH..."

if [ ! -d "$INSTALL_PATH" ]; then
    mkdir -p "$INSTALL_PATH"
fi

cp "$(dirname "$0")/compiler/defacto" "$INSTALL_PATH/defacto"
chmod +x "$INSTALL_PATH/defacto"

echo "✅ Installation complete!"
echo ""
echo "================================"
echo "Usage:"
echo "  defacto -terminal hello.de"
echo "  defacto -kernel -o kernel.bin os.de"
echo "  defacto -h"
echo "================================"

# Verify installation
if command -v defacto &> /dev/null; then
    echo "✅ defacto is in PATH"
    defacto -h | head -5
else
    echo "⚠️  defacto not in PATH. Add to ~/.bashrc or ~/.zshrc:"
    echo "  export PATH=\"$INSTALL_PATH:\$PATH\""
fi
