#!/bin/bash
# Defacto Compiler Installer for macOS and Linux
# Version 0.30

set -e

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Default install path
INSTALL_DIR="$HOME/.local/bin"
COMPILER_DIR="$(cd "$(dirname "$0")" && pwd)/compiler"

echo "========================================"
echo "  Defacto Compiler Installer v0.30"
echo "========================================"
echo ""

# Check for required tools
check_dependencies() {
    echo -n "Checking dependencies... "
    
    MISSING=""
    
    # Check for g++
    if ! command -v g++ &> /dev/null; then
        MISSING="$MISSING g++"
    fi
    
    # Check for make
    if ! command -v make &> /dev/null; then
        MISSING="$MISSING make"
    fi
    
    # Check for nasm
    if ! command -v nasm &> /dev/null; then
        MISSING="$MISSING nasm"
    fi
    
    if [ -n "$MISSING" ]; then
        echo -e "${YELLOW}WARNING${NC}"
        echo ""
        echo "Missing dependencies:$MISSING"
        echo ""
        echo "Install them with:"
        echo ""
        echo "  macOS:"
        echo "    xcode-select --install"
        echo "    brew install nasm make"
        echo ""
        echo "  Ubuntu/Debian:"
        echo "    sudo apt update"
        echo "    sudo apt install build-essential nasm make"
        echo ""
        read -p "Continue anyway? (y/N) " -n 1 -r
        echo
        if [[ ! $REPLY =~ ^[Yy]$ ]]; then
            exit 1
        fi
    else
        echo -e "${GREEN}OK${NC}"
    fi
}

# Build compiler
build_compiler() {
    echo ""
    echo "Building compiler..."
    
    if [ ! -d "$COMPILER_DIR" ]; then
        echo -e "${RED}Error:${NC} Compiler directory not found: $COMPILER_DIR"
        exit 1
    fi
    
    cd "$COMPILER_DIR"
    
    if make; then
        echo -e "${GREEN}Build successful!${NC}"
    else
        echo -e "${RED}Build failed!${NC}"
        exit 1
    fi
    
    cd - > /dev/null
}

# Install to system
install_compiler() {
    echo ""
    echo "Installing to $INSTALL_DIR..."
    
    # Create install directory
    mkdir -p "$INSTALL_DIR"
    
    # Copy compiler
    cp "$COMPILER_DIR/defacto" "$INSTALL_DIR/"
    
    # Make executable
    chmod +x "$INSTALL_DIR/defacto"
    
    echo -e "${GREEN}Installation complete!${NC}"
}

# Update PATH
update_path() {
    echo ""
    echo "Updating PATH..."
    
    SHELL_RC=""
    
    # Detect shell
    if [ -f "$HOME/.bashrc" ]; then
        SHELL_RC="$HOME/.bashrc"
    elif [ -f "$HOME/.zshrc" ]; then
        SHELL_RC="$HOME/.zshrc"
    elif [ -f "$HOME/.bash_profile" ]; then
        SHELL_RC="$HOME/.bash_profile"
    fi
    
    if [ -n "$SHELL_RC" ]; then
        if ! grep -q "$INSTALL_DIR" "$SHELL_RC" 2>/dev/null; then
            echo "" >> "$SHELL_RC"
            echo "# Defacto Compiler" >> "$SHELL_RC"
            echo "export PATH=\"$INSTALL_DIR:\$PATH\"" >> "$SHELL_RC"
            echo -e "${GREEN}Added to $SHELL_RC${NC}"
            echo ""
            echo "Please restart your terminal or run:"
            echo "  source $SHELL_RC"
        else
            echo -e "${YELLOW}PATH already configured${NC}"
        fi
    else
        echo -e "${YELLOW}Could not detect shell config file${NC}"
        echo "Please add manually to your PATH:"
        echo "  export PATH=\"$INSTALL_DIR:\$PATH\""
    fi
}

# Verify installation
verify_installation() {
    echo ""
    echo "Verifying installation..."
    
    if [ -f "$INSTALL_DIR/defacto" ]; then
        echo -e "${GREEN}Compiler installed at: $INSTALL_DIR/defacto${NC}"
        
        echo ""
        echo "Testing compiler..."
        if "$INSTALL_DIR/defacto" -h 2>&1 | head -5; then
            echo ""
            echo -e "${GREEN}========================================${NC}"
            echo -e "${GREEN}  Installation Successful!${NC}"
            echo -e "${GREEN}========================================${NC}"
            echo ""
            echo "Quick start:"
            echo "  1. Restart your terminal or run: source $SHELL_RC"
            echo "  2. Compile your first program:"
            echo "     defacto -terminal hello.de"
            echo "  3. Run it:"
            echo "     ./hello"
            echo ""
            echo "For more information, visit:"
            echo "  https://github.com/vivooifo-droid/Defacto"
            echo ""
        else
            echo -e "${RED}Compiler test failed!${NC}"
            exit 1
        fi
    else
        echo -e "${RED}Installation verification failed!${NC}"
        exit 1
    fi
}

# Main
main() {
    check_dependencies
    build_compiler
    install_compiler
    update_path
    verify_installation
}

# Handle custom install path
while [[ $# -gt 0 ]]; do
    case $1 in
        --prefix)
            INSTALL_DIR="$2"
            shift 2
            ;;
        --help)
            echo "Usage: $0 [--prefix /path/to/install]"
            echo ""
            echo "Options:"
            echo "  --prefix PATH  Install to specified path (default: \$HOME/.local/bin)"
            echo "  --help         Show this help message"
            exit 0
            ;;
        *)
            echo "Unknown option: $1"
            exit 1
            ;;
    esac
done

main
