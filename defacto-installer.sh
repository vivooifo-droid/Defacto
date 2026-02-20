#!/bin/bash
# Defacto Compiler Installer - Universal (macOS/Linux)
# This is the file people download and run directly
# Usage: bash defacto-installer.sh
#   or chmod +x defacto-installer.sh && ./defacto-installer.sh

set -e

REPO_URL="https://github.com/vivooifo-droid/Defacto.git"
INSTALL_PATH="${HOME}/.local/bin"
TEMP_DIR=$(mktemp -d)

cleanup() {
    rm -rf "$TEMP_DIR"
}
trap cleanup EXIT

show_banner() {
    echo ""
    echo "╔════════════════════════════════════╗"
    echo "║  Defacto Compiler Installer       ║"
    echo "║  https://github.com/vivooifo-droid║"
    echo "╚════════════════════════════════════╝"
    echo ""
}

check_deps() {
    local missing=()
    
    echo "📋 Checking dependencies..."
    
    for cmd in git g++ make; do
        if command -v "$cmd" &> /dev/null; then
            echo "  ✅ $cmd"
        else
            echo "  ❌ $cmd"
            missing+=("$cmd")
        fi
    done
    
    if [ ${#missing[@]} -gt 0 ]; then
        echo ""
        echo "❌ Missing: ${missing[*]}"
        echo ""
        if [[ "$OSTYPE" == "darwin"* ]]; then
            echo "Install with: brew install ${missing[*]}"
        else
            echo "Install with: sudo apt install ${missing[*]}"
        fi
        exit 1
    fi
    
    echo "✅ All dependencies OK"
    echo ""
}

download_repo() {
    echo "📥 Downloading Defacto..."
    git clone --depth 1 "$REPO_URL" "$TEMP_DIR" > /dev/null 2>&1
    echo "✅ Downloaded"
    echo ""
}

build() {
    echo "📦 Building compiler..."
    cd "$TEMP_DIR/compiler"
    make clean > /dev/null 2>&1
    make > /dev/null 2>&1
    echo "✅ Built"
    echo ""
}

install() {
    echo "📁 Installing..."
    
    mkdir -p "$INSTALL_PATH"
    cp "$TEMP_DIR/compiler/defacto" "$INSTALL_PATH/defacto"
    chmod +x "$INSTALL_PATH/defacto"
    
    echo "✅ Installed to $INSTALL_PATH"
    echo ""
    
    # Check if in PATH
    if ! echo "$PATH" | grep -q "$INSTALL_PATH"; then
        echo "⚠️  Add to PATH by adding to ~/.bashrc or ~/.zshrc:"
        echo "   export PATH=\"\$HOME/.local/bin:\$PATH\""
        echo ""
    fi
}

verify() {
    if command -v defacto &> /dev/null; then
        echo "✅ defacto is ready!"
        echo ""
        defacto -h | head -8
    else
        echo "⚠️  Please restart terminal or add $INSTALL_PATH to PATH"
    fi
}

main() {
    show_banner
    check_deps
    download_repo
    build
    install
    
    echo "╔════════════════════════════════════╗"
    echo "║  ✅ Installation Complete!         ║"
    echo "╚════════════════════════════════════╝"
    echo ""
    
    verify
}

main
