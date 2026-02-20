#!/usr/bin/env python3
"""
Defacto Compiler - One-line Installer
Download, build, and install in one command

Usage:
  python3 -c "$(curl -fsSL https://raw.githubusercontent.com/defacto-lang/defacto/main/quick-install.py)"
  
Or locally:
  python3 quick-install.py
"""

import os
import sys
import subprocess
import shutil
import tempfile
from pathlib import Path

def run_cmd(cmd, check=True):
    """Run command and return result"""
    result = subprocess.run(cmd, shell=True, capture_output=True, text=True)
    if check and result.returncode != 0:
        print(f"❌ Error: {result.stderr}")
        sys.exit(1)
    return result

def check_command(cmd):
    """Check if command exists"""
    return shutil.which(cmd) is not None

def main():
    print("\n" + "="*50)
    print("Defacto Compiler - Quick Installer")
    print("="*50 + "\n")
    
    # Check OS
    is_windows = sys.platform.startswith('win')
    is_macos = sys.platform == 'darwin'
    is_linux = sys.platform.startswith('linux')
    
    print(f"🖥️  Detected: ", end="")
    if is_windows:
        print("Windows")
    elif is_macos:
        print("macOS")
    elif is_linux:
        print("Linux")
    print()
    
    # Check dependencies
    print("📋 Checking dependencies...\n")
    
    deps = ['g++', 'make']
    if not is_windows:
        deps.append('nasm')
    
    missing = []
    for dep in deps:
        if check_command(dep):
            print(f"  ✅ {dep}")
        else:
            print(f"  ❌ {dep} - NOT FOUND")
            missing.append(dep)
    
    if missing:
        print(f"\n❌ Missing dependencies: {', '.join(missing)}")
        print("\nInstall with:")
        if is_macos:
            print("  brew install", " ".join(missing))
        elif is_linux:
            print("  sudo apt install", " ".join(missing))
        elif is_windows:
            print("  choco install", " ".join(missing))
        sys.exit(1)
    
    print("\n✅ All dependencies OK\n")
    
    # Clone or use existing repo
    script_dir = Path(__file__).parent
    
    if (script_dir / 'compiler' / 'main.cpp').exists():
        print("📂 Using local repository")
        repo_dir = script_dir
    else:
        print("📥 Downloading Defacto from GitHub...")
        with tempfile.TemporaryDirectory() as tmpdir:
            tmpdir = Path(tmpdir)
            repo_url = "https://github.com/vivooifo-droid/Defacto.git"
            run_cmd(f"git clone --depth 1 {repo_url} {tmpdir}")
            repo_dir = tmpdir
            _build_and_install(repo_dir, is_windows)
        return
    
    _build_and_install(repo_dir, is_windows)

def _build_and_install(repo_dir, is_windows):
    """Build and install compiler"""
    compiler_dir = repo_dir / 'compiler'
    
    print(f"📦 Building compiler...")
    os.chdir(compiler_dir)
    
    run_cmd('make clean')
    if is_windows:
        run_cmd('make windows')
        exe = compiler_dir / 'defacto.exe'
    else:
        run_cmd('make')
        exe = compiler_dir / 'defacto'
    
    print(f"✅ Built: {exe.name}\n")
    
    # Install
    if is_windows:
        install_path = Path(os.environ['ProgramFiles']) / 'Defacto'
    else:
        install_path = Path('/usr/local/bin')
    
    print(f"📁 Installing to {install_path}...")
    
    if not install_path.exists():
        install_path.mkdir(parents=True, exist_ok=True)
    
    target = install_path / exe.name
    shutil.copy(exe, target)
    target.chmod(0o755)
    
    print(f"✅ Installed: {target}\n")
    
    # Add to PATH (Windows)
    if is_windows:
        env_path = os.environ.get('PATH', '')
        if str(install_path) not in env_path:
            os.system(f'setx PATH "%PATH%;{install_path}"')
            print("✅ Added to PATH (restart terminal)\n")
    
    print("="*50)
    print("✅ Installation Complete!")
    print("="*50)
    print("\nUsage:")
    print("  defacto -h                    # Help")
    print("  defacto -terminal hello.de    # Compile")
    print("  defacto -kernel -o out.bin os.de")
    print("\nDon't forget NASM:")
    if is_windows:
        print("  choco install nasm")
    elif is_macos:
        print("  brew install nasm")
    else:
        print("  sudo apt install nasm")
    print()

if __name__ == '__main__':
    main()
