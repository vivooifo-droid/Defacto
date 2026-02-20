#!/usr/bin/env python3
"""
Defacto Windows Installer Builder
Создаёт готовый .exe установщик для Windows
"""

import os
import subprocess
import sys
import shutil
from pathlib import Path

def check_requirements():
    """Проверить наличие необходимых инструментов"""
    requirements = {
        'x86_64-w64-mingw32-g++': 'MinGW-w64 cross-compiler',
        'makefile': 'Make utility',
        'nsis': 'NSIS installer (required on Windows to build installer)',
    }
    
    missing = []
    for cmd, desc in requirements.items():
        if cmd == 'makefile':
            continue
        if not shutil.which(cmd):
            if cmd != 'nsis':  # NSIS warning only
                missing.append(f"  - {desc} ({cmd})")
    
    if missing:
        print("❌ Missing dependencies:")
        for m in missing:
            print(m)
        return False
    
    return True

def build_compiler():
    """Собрать компилятор для Windows"""
    print("\n📦 Building Defacto compiler for Windows...")
    
    os.chdir('compiler')
    result = subprocess.run(['make', 'windows'], capture_output=True, text=True)
    os.chdir('..')
    
    if result.returncode != 0:
        print("❌ Compiler build failed:")
        print(result.stderr)
        return False
    
    print("✅ Compiler built: compiler/defacto.exe")
    return True

def create_installer_package():
    """Создать структуру для установщика"""
    print("\n📁 Preparing installer package...")
    
    pkg_dir = Path('installer_pkg')
    pkg_dir.mkdir(exist_ok=True)
    
    # Copy files
    shutil.copy('compiler/defacto.exe', pkg_dir / 'defacto.exe')
    shutil.copy('hello.de', pkg_dir / 'hello.de')
    shutil.copy('README.md', pkg_dir / 'README.txt')
    
    print("✅ Package prepared in ./installer_pkg/")
    return pkg_dir

def create_nsis_script():
    """Создать NSIS скрипт установщика"""
    print("\n🔧 Creating NSIS installer script...")
    
    nsis_content = """
; Defacto Compiler Windows Installer
!include "MUI2.nsh"

Name "Defacto Compiler v0.25"
OutFile "defacto-0.25-installer.exe"
InstallDir "$PROGRAMFILES\\Defacto"
RequestExecutionLevel admin

!insertmacro MUI_PAGE_DIRECTORY
!insertmacro MUI_PAGE_INSTFILES
!insertmacro MUI_LANGUAGE "English"

Section "Install"
  SetOutPath "$INSTDIR"
  File "defacto.exe"
  
  CreateDirectory "$INSTDIR\\examples"
  File "/oname=examples\\hello.de" "hello.de"
  
  CreateDirectory "$INSTDIR\\docs"
  File "/oname=docs\\README.txt" "README.txt"
  
  ; Add to PATH
  ReadRegStr $0 HKCU "Environment" "PATH"
  ${If} $0 == ""
    StrCpy $0 "$INSTDIR"
  ${Else}
    StrCpy $0 "$0;$INSTDIR"
  ${EndIf}
  WriteRegExpandStr HKCU "Environment" "PATH" $0
  
  CreateDirectory "$SMPROGRAMS\\Defacto"
  CreateShortCut "$SMPROGRAMS\\Defacto\\Defacto.lnk" "$INSTDIR\\defacto.exe"
  CreateShortCut "$SMPROGRAMS\\Defacto\\Uninstall.lnk" "$INSTDIR\\uninstall.exe"
  
  WriteUninstaller "$INSTDIR\\uninstall.exe"
  MessageBox MB_OK "Defacto Compiler installed!\\n\\nDon't forget to install NASM:\\nhttps://www.nasm.us"
SectionEnd

Section "Uninstall"
  Delete "$INSTDIR\\defacto.exe"
  Delete "$INSTDIR\\uninstall.exe"
  RMDir /r "$INSTDIR"
  RMDir /r "$SMPROGRAMS\\Defacto"
SectionEnd
"""
    
    nsis_path = Path('installer_pkg/installer.nsi')
    nsis_path.write_text(nsis_content)
    print(f"✅ NSIS script created: {nsis_path}")
    return nsis_path

def build_installer():
    """Собрать .exe установщик"""
    print("\n⚙️  Building installer with NSIS...")
    
    # Check if we're on Windows and NSIS is available
    if shutil.which('makensis'):
        os.chdir('installer_pkg')
        result = subprocess.run(['makensis', 'installer.nsi'], capture_output=True, text=True)
        os.chdir('..')
        
        if result.returncode == 0:
            exe = Path('installer_pkg/defacto-0.25-installer.exe')
            if exe.exists():
                print(f"✅ Installer created: {exe.absolute()}")
                return True
        else:
            print("❌ NSIS build failed:")
            print(result.stderr)
            return False
    else:
        print("⚠️  NSIS not found on this system.")
        print("   To build installer on Windows, download NSIS:")
        print("   https://nsis.sourceforge.io/download/")
        print("\n   Then run from Windows:")
        print("   makensis installer_pkg/installer.nsi")
        return False

def main():
    print("=" * 60)
    print("Defacto Windows Installer Builder")
    print("=" * 60)
    
    # For now, just build compiler
    if not build_compiler():
        sys.exit(1)
    
    # Create package
    pkg = create_installer_package()
    
    # Create NSIS script
    create_nsis_script()
    
    # Try to build installer
    print("\n" + "=" * 60)
    print("INSTALLER CREATION")
    print("=" * 60)
    
    build_installer()
    
    print("\n" + "=" * 60)
    print("✅ Build complete!")
    print("=" * 60)
    print("\nNext steps:")
    print("1. Install NASM: https://www.nasm.us/")
    print("2. On Windows, run: makensis installer_pkg/installer.nsi")
    print("3. Share: defacto-0.25-installer.exe")

if __name__ == '__main__':
    main()
