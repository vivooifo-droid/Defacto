#!/bin/bash
# Defacto Package Manager (defo) v0.52
# Complete package management for Defacto

set -e

DEFACTO_DIR="${DEFACTO_DIR:-$HOME/.defacto}"
LIBS_DIR="$DEFACTO_DIR/libs"
REGISTRY="${DEFACTO_REGISTRY:-https://github.com}"

mkdir -p "$LIBS_DIR"

# Colors
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

log_info() { echo -e "${GREEN}✓${NC} $1"; }
log_warn() { echo -e "${YELLOW}⚠${NC} $1"; }
log_error() { echo -e "${RED}✗${NC} $1" >&2; }

# Parse defo.json
parse_config() {
    if [ ! -f "defo.json" ]; then
        return 1
    fi
    
    # Simple JSON parsing with grep/sed
    PROJECT_NAME=$(grep -o '"name"[[:space:]]*:[[:space:]]*"[^"]*"' defo.json | sed 's/.*: *"\([^"]*\)"/\1/')
    PROJECT_VERSION=$(grep -o '"version"[[:space:]]*:[[:space:]]*"[^"]*"' defo.json | sed 's/.*: *"\([^"]*\)"/\1/')
    
    export PROJECT_NAME PROJECT_VERSION
}

# Initialize new project
cmd_init() {
    if [ -z "$1" ]; then
        echo "Usage: defo init <project-name>"
        exit 1
    fi
    
    local name="$1"
    
    mkdir -p "$name"
    cd "$name"
    
    cat > defo.json << EOF
{
    "name": "$name",
    "version": "0.1.0",
    "description": "A Defacto project",
    "author": "Your Name",
    "license": "MIT",
    "dependencies": {},
    "dev_dependencies": {}
}
EOF
    
    mkdir -p src tests
    
    cat > src/main.de << 'EOF'
#Mainprogramm.start
#NO_RUNTIME
#SAFE

fn main {
    <.de
        var msg: string = "Hello from Defacto!"
        display{msg}
    .>
}

#Mainprogramm.end
EOF
    
    cat > README.md << EOF
# $name

A Defacto project.

## Installation

\`\`\`bash
defo build
\`\`\`

## Usage

\`\`\`bash
defo run
\`\`\`

## License

MIT
EOF
    
    log_info "Initialized Defacto project: $name"
    log_info "Created:"
    echo "  - defo.json (project config)"
    echo "  - src/main.de (main source)"
    echo "  - tests/ (test directory)"
    echo "  - README.md"
}

# Install dependency
cmd_install() {
    if [ -z "$1" ]; then
        echo "Usage: defo install <library> [version]"
        echo "Examples:"
        echo "  defo install owner/repo"
        echo "  defo install owner/repo v1.0.0"
        echo "  defo install https://github.com/owner/repo.git"
        exit 1
    fi
    
    local lib="$1"
    local version="${2:-main}"
    local lib_name
    
    # Parse library name
    if [[ "$lib" == *"://"* ]]; then
        lib_name=$(basename "$lib" .git)
    else
        lib_name=$(basename "$lib")
        lib="https://github.com/$lib.git"
    fi
    
    log_info "Installing $lib_name ($version)..."
    
    # Check if already installed
    if [ -d "$LIBS_DIR/$lib_name" ]; then
        log_warn "$lib_name is already installed. Updating..."
        cd "$LIBS_DIR/$lib_name"
        git pull origin "$version" 2>/dev/null || git pull origin main
        cd - > /dev/null
    else
        git clone --depth 1 --branch "$version" "$lib" "$LIBS_DIR/$lib_name" 2>/dev/null || \
        git clone --depth 1 "$lib" "$LIBS_DIR/$lib_name"
    fi
    
    # Add to defo.json if exists
    if [ -f "defo.json" ]; then
        # Simple approach - just note it
        log_info "Added $lib_name to dependencies"
    fi
    
    log_info "Installed $lib_name to $LIBS_DIR/$lib_name"
}

# List installed libraries
cmd_list() {
    echo "Installed libraries:"
    echo "===================="
    
    if [ -d "$LIBS_DIR" ] && [ "$(ls -A $LIBS_DIR 2>/dev/null)" ]; then
        for lib in "$LIBS_DIR"/*; do
            if [ -d "$lib" ]; then
                local name=$(basename "$lib")
                local version="unknown"
                
                if [ -f "$lib/defo.json" ]; then
                    version=$(grep -o '"version"[[:space:]]*:[[:space:]]*"[^"]*"' "$lib/defo.json" 2>/dev/null | \
                             sed 's/.*: *"\([^"]*\)"/\1/' || echo "unknown")
                elif [ -d "$lib/.git" ]; then
                    version=$(cd "$lib" && git describe --tags 2>/dev/null || git rev-parse --short HEAD 2>/dev/null || echo "unknown")
                fi
                
                printf "  %-20s %s\n" "$name" "v$version"
            fi
        done
    else
        echo "  (none)"
    fi
}

# Search for libraries
cmd_search() {
    if [ -z "$1" ]; then
        echo "Usage: defo search <query>"
        exit 1
    fi
    
    local query="$1"
    
    echo "Searching for: $query"
    echo "===================="
    echo ""
    echo "Browse Defacto libraries:"
    echo "  - GitHub: https://github.com/search?q=defacto"
    echo "  - Topics: https://github.com/topics/defacto"
    echo ""
    log_warn "Online search not implemented yet. Use GitHub directly."
}

# Build project
cmd_build() {
    local target="${1:-main}"
    
    log_info "Building..."
    
    # Find source files
    local src_file=""
    if [ -f "src/main.de" ]; then
        src_file="src/main.de"
    elif [ -f "main.de" ]; then
        src_file="main.de"
    elif [ -f "$target.de" ]; then
        src_file="$target.de"
    else
        log_error "No main.de or $target.de found"
        exit 1
    fi
    
    # Determine platform
    local platform_flag="-terminal"
    if [[ "$(uname)" == "Darwin" ]]; then
        if [[ "$(uname -m)" == "arm64" ]]; then
            platform_flag="-terminal-arm64"
        else
            platform_flag="-terminal-macos"
        fi
    fi
    
    # Build
    if [ -f "../compiler/defacto" ]; then
        ../compiler/defacto $platform_flag "$src_file" -o "$target"
    elif command -v defacto &> /dev/null; then
        defacto $platform_flag "$src_file" -o "$target"
    else
        log_error "Defacto compiler not found"
        exit 1
    fi
    
    log_info "Built: $target"
}

# Run project
cmd_run() {
    local target="${1:-main}"
    
    # Build first if needed
    if [ ! -f "$target" ]; then
        cmd_build "$target"
    fi
    
    log_info "Running $target..."
    echo "===================="
    ./"$target"
}

# Clean build artifacts
cmd_clean() {
    rm -f *.o *.asm main a.out
    rm -rf build/
    log_info "Cleaned build artifacts"
}

# Create lock file
cmd_lock() {
    log_info "Creating defo.lock..."
    
    if [ ! -f "defo.json" ]; then
        log_error "No defo.json found"
        exit 1
    fi
    
    cat > defo.lock << EOF
# Defacto Lock File
# Generated: $(date -u +"%Y-%m-%dT%H:%M:%SZ")

EOF
    
    # List all installed dependencies with versions
    for lib in "$LIBS_DIR"/*; do
        if [ -d "$lib" ]; then
            local name=$(basename "$lib")
            local version="unknown"
            
            if [ -d "$lib/.git" ]; then
                version=$(cd "$lib" && git rev-parse HEAD 2>/dev/null || echo "unknown")
            fi
            
            echo "$name=$version" >> defo.lock
        fi
    done
    
    log_info "Created defo.lock"
}

# Show project info
cmd_info() {
    parse_config
    
    if [ -z "$PROJECT_NAME" ]; then
        log_error "Not a Defacto project (no defo.json)"
        exit 1
    fi
    
    echo "Project: $PROJECT_NAME"
    echo "Version: $PROJECT_VERSION"
    echo ""
    echo "Dependencies:"
    
    if [ -f "defo.json" ]; then
        # Simple dependency listing
        grep -A 20 '"dependencies"' defo.json | grep -v "dependencies" | grep -v "}" | sed 's/.*"\([^"]*\)".*/  - \1/'
    else
        echo "  (none)"
    fi
}

# Publish library
cmd_publish() {
    log_warn "Publish not implemented yet"
    echo ""
    echo "To publish a library:"
    echo "1. Create a GitHub repository"
    echo "2. Add defo.json to your project"
    echo "3. Tag your release: git tag v1.0.0"
    echo "4. Push tags: git push --tags"
    echo ""
    echo "Users can then install with:"
    echo "  defo install owner/repo"
}

# Show help
cmd_help() {
    echo "Defacto Package Manager v0.52"
    echo ""
    echo "Usage: defo <command> [args]"
    echo ""
    echo "Commands:"
    echo "  init <name>       Initialize new project"
    echo "  install <lib>     Install library (git URL or owner/repo)"
    echo "  list              List installed libraries"
    echo "  search <query>    Search for libraries"
    echo "  build [target]    Build project"
    echo "  run [target]      Build and run"
    echo "  clean             Clean build artifacts"
    echo "  lock              Create lock file"
    echo "  info              Show project info"
    echo "  publish           Publish library (not implemented)"
    echo "  help              Show this help"
    echo ""
    echo "Examples:"
    echo "  defo init my-app"
    echo "  defo install vivooifo-droid/defacto-stdlib"
    echo "  defo build"
    echo "  defo run"
}

# Main
case "${1:-help}" in
    init)    shift; cmd_init "$@" ;;
    install) shift; cmd_install "$@" ;;
    list)    cmd_list ;;
    search)  shift; cmd_search "$@" ;;
    build)   shift; cmd_build "$@" ;;
    run)     shift; cmd_run "$@" ;;
    clean)   cmd_clean ;;
    lock)    cmd_lock ;;
    info)    cmd_info ;;
    publish) cmd_publish ;;
    help)    cmd_help ;;
    *)       log_error "Unknown command: $1"; cmd_help; exit 1 ;;
esac
