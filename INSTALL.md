# Installation Guide - Defacto Compiler

## Quick Install

### macOS / Linux

```bash
bash install.sh
```

Optional - specify custom install path:
```bash
bash install.sh --prefix /custom/path
```

### Windows (PowerShell)

```powershell
powershell -ExecutionPolicy Bypass -File install.ps1
```

Or right-click `install.ps1` → "Run with PowerShell"

---

## What the installers do

✅ Check dependencies (g++, make, nasm)  
✅ Build compiler from source  
✅ Install to system path  
✅ Make `defacto` command available globally  
✅ Create Start Menu shortcuts (Windows)  
✅ Verify installation  

---

## Requirements by OS

### macOS
```bash
xcode-select --install      # Xcode command line tools
brew install nasm make      # Build tools
```

### Linux (Ubuntu/Debian)
```bash
sudo apt update
sudo apt install build-essential nasm make g++
```

### Windows
- Visual Studio Community OR MinGW-w64
- NASM: `choco install nasm` or https://www.nasm.us/
- Make: `choco install make` (optional, if not in Visual Studio)

---

## Usage after install

```bash
# Check installation
defacto -h

# Compile to terminal (Linux syscalls)
defacto -terminal program.de

# Compile bare-metal (x86-32)
defacto -kernel -o kernel.bin os.de

# Assembly only (don't assemble)
defacto -S program.de

# Verbose output
defacto -v program.de
```

---

## Troubleshooting

### "command not found: defacto"

**macOS/Linux:**
```bash
# Add to ~/.bashrc or ~/.zshrc:
export PATH="/usr/local/bin:$PATH"

# Then restart terminal or run:
source ~/.bashrc
```

**Windows:**
- Close and reopen PowerShell/Command Prompt
- Or restart Windows

### "make: command not found"

**macOS:**
```bash
brew install make
```

**Linux:**
```bash
sudo apt install make
```

**Windows:**
```powershell
choco install make
```

### "nasm: command not found"

Install NASM:
- https://www.nasm.us/
- macOS: `brew install nasm`
- Linux: `sudo apt install nasm`
- Windows: `choco install nasm`

---

## Manual Installation

If scripts don't work:

```bash
cd compiler
make           # macOS/Linux
# or
make windows   # Cross-compile for Windows

cp defacto /usr/local/bin/
```

Then add to PATH if needed.

---

## Uninstall

**macOS/Linux:**
```bash
rm /usr/local/bin/defacto
```

**Windows:**
- Go to: Programs and Features → Uninstall
- Or remove from: `C:\Program Files\Defacto`
- Or run PowerShell installer again to update
