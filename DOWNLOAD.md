# Download & Install Defacto

## Windows

1. **Download:** [defacto-0.30-installer.exe](https://github.com/vivooifo-droid/Defacto/releases/download/v0.30/defacto-0.30-installer.exe)
2. **Right-click** → "Run as administrator"
3. **Wait** for installation
4. **Done!** ✅

Then install NASM:
```powershell
choco install nasm
```

Or download from: https://www.nasm.us/

---

## macOS / Linux

### Option 1: Direct download
1. Download: [defacto-installer.sh](https://github.com/vivooifo-droid/Defacto/releases/download/latest/defacto-installer.sh)
2. Open terminal
3. Run:
```bash
bash ~/Downloads/defacto-installer.sh
```

### Option 2: One-line (direct from GitHub)
```bash
bash -c "$(curl -fsSL https://raw.githubusercontent.com/vivooifo-droid/Defacto/main/defacto-installer.sh)"
```

Then install NASM:
```bash
# macOS
brew install nasm

# Linux
sudo apt install nasm
```

---

## Usage

After installation, you can use:

```bash
# Show help
defacto -h

# Compile to terminal mode (Linux syscalls)
defacto -terminal program.de

# Compile to bare-metal (x86-32)
defacto -kernel -o kernel.bin os.de

# Assembly only (don't assemble)
defacto -S program.de

# Verbose output
defacto -v program.de
```

---

## Troubleshooting

### "command not found: defacto" on macOS/Linux
Add to `~/.bashrc` or `~/.zshrc`:
```bash
export PATH="$HOME/.local/bin:$PATH"
```

Then restart terminal or run:
```bash
source ~/.bashrc
```

### "g++ command not found"
Install compiler:
- **macOS:** `xcode-select --install`
- **Linux:** `sudo apt install build-essential`
- **Windows:** Visual Studio Community

### "make: command not found"
- **macOS:** `brew install make`
- **Linux:** `sudo apt install make`
- **Windows:** `choco install make` or use Visual Studio

---

## Having Issues?

1. Check all dependencies are installed
2. Restart terminal/command prompt
3. Verify: `defacto -h`
4. Report issues: https://github.com/vivooifo-droid/Defacto/issues

**Enjoy!** 🚀
