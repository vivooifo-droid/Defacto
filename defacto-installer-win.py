#!/usr/bin/env python3
"""
Defacto Compiler Installer for Windows
Compile to .exe with: pyinstaller --onefile --windowed defacto-installer-win.py
Or: pyinstaller --onefile --icon=icon.ico defacto-installer-win.py
"""

import os
import sys
import subprocess
import shutil
import tempfile
from pathlib import Path
import tkinter as tk
from tkinter import messagebox
import threading

class DefactoInstaller:
    def __init__(self):
        self.root = tk.Tk()
        self.root.title("Defacto Compiler Installer")
        self.root.geometry("500x400")
        self.root.resizable(False, False)
        
        # Center window
        self.root.update_idletasks()
        x = (self.root.winfo_screenwidth() // 2) - (self.root.winfo_width() // 2)
        y = (self.root.winfo_screenheight() // 2) - (self.root.winfo_height() // 2)
        self.root.geometry(f"+{x}+{y}")
        
        self.setup_ui()
    
    def setup_ui(self):
        # Title
        title = tk.Label(
            self.root,
            text="Defacto Compiler",
            font=("Arial", 24, "bold"),
            fg="#0066cc"
        )
        title.pack(pady=20)
        
        # Description
        desc = tk.Label(
            self.root,
            text="Low-level programming language for x86-32\n\nThis installer will:",
            font=("Arial", 10),
            justify=tk.LEFT
        )
        desc.pack(pady=10, padx=20)
        
        # Features
        features = tk.Label(
            self.root,
            text="✓ Download from GitHub\n✓ Build compiler\n✓ Add to PATH\n✓ Create shortcuts",
            font=("Arial", 9),
            justify=tk.LEFT,
            fg="#333"
        )
        features.pack(pady=10, padx=40)
        
        # Info
        info = tk.Label(
            self.root,
            text="Requirements: git, g++, make",
            font=("Arial", 8),
            fg="#666"
        )
        info.pack(pady=10)
        
        # Progress
        self.progress_label = tk.Label(
            self.root,
            text="Ready to install",
            font=("Arial", 9),
            fg="#333"
        )
        self.progress_label.pack(pady=10)
        
        # Install button
        self.install_btn = tk.Button(
            self.root,
            text="Install Now",
            font=("Arial", 12, "bold"),
            bg="#0066cc",
            fg="white",
            padx=40,
            pady=10,
            command=self.start_install
        )
        self.install_btn.pack(pady=20)
        
        # Exit button
        self.exit_btn = tk.Button(
            self.root,
            text="Exit",
            font=("Arial", 10),
            fg="#666",
            command=self.root.quit
        )
        self.exit_btn.pack(pady=5)
    
    def start_install(self):
        self.install_btn.config(state=tk.DISABLED)
        self.exit_btn.config(state=tk.DISABLED)
        
        thread = threading.Thread(target=self.install)
        thread.daemon = True
        thread.start()
    
    def update_progress(self, msg):
        self.progress_label.config(text=msg)
        self.root.update()
    
    def install(self):
        try:
            self.update_progress("📋 Checking dependencies...")
            if not self.check_deps():
                return
            
            self.update_progress("📥 Downloading repository...")
            repo_dir = self.download_repo()
            
            self.update_progress("📦 Building compiler...")
            if not self.build(repo_dir):
                return
            
            self.update_progress("📁 Installing...")
            if not self.install_compiler(repo_dir):
                return
            
            self.update_progress("✅ Installation complete!")
            messagebox.showinfo("Success", "Defacto installed successfully!\n\nDon't forget to install NASM:\nhttps://www.nasm.us/\n\nor: choco install nasm")
            
        except Exception as e:
            messagebox.showerror("Error", f"Installation failed:\n{str(e)}")
        finally:
            self.install_btn.config(state=tk.NORMAL)
            self.exit_btn.config(state=tk.NORMAL)
    
    def check_command(self, cmd):
        return shutil.which(cmd) is not None
    
    def check_deps(self):
        deps = ['git', 'g++', 'make']
        missing = [d for d in deps if not self.check_command(d)]
        
        if missing:
            msg = f"Missing dependencies:\n\n"
            for dep in missing:
                msg += f"• {dep}\n"
            msg += f"\nPlease install using: choco install {' '.join(missing)}"
            messagebox.showerror("Missing Dependencies", msg)
            return False
        
        return True
    
    def download_repo(self):
        temp_dir = tempfile.mkdtemp()
        repo_url = "https://github.com/vivooifo-droid/Defacto.git"
        
        result = subprocess.run(
            f'git clone --depth 1 "{repo_url}" "{temp_dir}"',
            shell=True,
            capture_output=True,
            text=True
        )
        
        if result.returncode != 0:
            raise Exception("Failed to download repository")
        
        return temp_dir
    
    def build(self, repo_dir):
        compiler_dir = Path(repo_dir) / 'compiler'
        
        try:
            os.chdir(compiler_dir)
            subprocess.run('make clean', shell=True, capture_output=True)
            result = subprocess.run('make windows', shell=True, capture_output=True, text=True)
            
            if result.returncode != 0 or not (compiler_dir / 'defacto.exe').exists():
                raise Exception("Build failed")
            
            return True
        except Exception as e:
            messagebox.showerror("Build Error", f"Compiler build failed:\n{str(e)}")
            return False
    
    def install_compiler(self, repo_dir):
        try:
            install_path = Path(os.environ['ProgramFiles']) / 'Defacto'
            install_path.mkdir(parents=True, exist_ok=True)
            
            exe_src = Path(repo_dir) / 'compiler' / 'defacto.exe'
            exe_dst = install_path / 'defacto.exe'
            
            shutil.copy(exe_src, exe_dst)
            
            # Add to PATH
            env_path = os.environ.get('PATH', '')
            if str(install_path) not in env_path:
                subprocess.run(
                    f'setx PATH "%PATH%;{install_path}"',
                    shell=True
                )
            
            # Cleanup
            shutil.rmtree(repo_dir, ignore_errors=True)
            
            return True
        except Exception as e:
            messagebox.showerror("Install Error", f"Installation failed:\n{str(e)}")
            return False
    
    def run(self):
        self.root.mainloop()

if __name__ == '__main__':
    if not sys.platform.startswith('win'):
        print("This installer is for Windows only")
        sys.exit(1)
    
    app = DefactoInstaller()
    app.run()
