#!/bin/bash
# Build Windows installer for Defacto Compiler
# Requires: mingw-w64, nsis (makensis)

set -e

VERSION="0.25"
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

# Step 3: Build NSIS installer
echo "🔨 Building NSIS installer..."
cd "$INSTALLER_PKG"

# Create updated NSIS script with correct paths
cat > installer.nsi << 'NSIS_EOF'
; Defacto Compiler Windows Installer
; Built with NSIS

!include "MUI2.nsh"

; General settings
Name "Defacto Compiler v0.25"
OutFile "defacto-0.25-installer.exe"
InstallDir "$PROGRAMFILES\Defacto"
InstallDirRegKey HKCU "Software\Defacto" "InstallDir"

; Request admin privileges
RequestExecutionLevel admin

; Icons
; Icon "icon.ico"
; UninstallIcon "icon.ico"

; UI Settings
!define MUI_ABORTWARNING
!define MUI_FINISHPAGE_SHOWREADME "$INSTDIR\docs\README.txt"
!define MUI_FINISHPAGE_RUN "$INSTDIR\defacto.exe"
!define MUI_FINISHPAGE_RUN_PARAMETERS "-h"

!insertmacro MUI_PAGE_WELCOME
!insertmacro MUI_PAGE_LICENSE "README.txt"
!insertmacro MUI_PAGE_DIRECTORY
!insertmacro MUI_PAGE_INSTFILES
!insertmacro MUI_PAGE_FINISH

!insertmacro MUI_LANGUAGE "English"
!insertmacro MUI_LANGUAGE "Russian"

; Installer sections
Section "Defacto Compiler (Required)"
  SectionIn RO
  
  SetOutPath "$INSTDIR"
  
  ; Copy main executable
  File "defacto.exe"
  
  ; Create examples directory
  SetOutPath "$INSTDIR\examples"
  File "hello.de"
  
  ; Create docs directory
  SetOutPath "$INSTDIR\docs"
  File "README.txt"
  
  ; Create uninstaller
  WriteUninstaller "$INSTDIR\uninstall.exe"
  
  ; Add to PATH
  Push "$INSTDIR"
  Call AddToPath
  
SectionEnd

Section "Create Start Menu Shortcuts"
  CreateDirectory "$SMPROGRAMS\Defacto"
  
  CreateShortCut "$SMPROGRAMS\Defacto\Defacto Compiler.lnk" "$INSTDIR\defacto.exe" "" "$INSTDIR\defacto.exe" 0
  CreateShortCut "$SMPROGRAMS\Defacto\Examples.lnk" "$INSTDIR\examples" "" "" 0
  CreateShortCut "$SMPROGRAMS\Defacto\Documentation.lnk" "$INSTDIR\docs\README.txt" "" "" 0
  CreateShortCut "$SMPROGRAMS\Defacto\Uninstall.lnk" "$INSTDIR\uninstall.exe" "" "$INSTDIR\uninstall.exe" 0
SectionEnd

Section "Create Desktop Shortcut"
  CreateShortCut "$DESKTOP\Defacto Compiler.lnk" "$INSTDIR\defacto.exe" "" "$INSTDIR\defacto.exe" 0
SectionEnd

; Uninstaller
Section "Uninstall"
  Delete "$INSTDIR\defacto.exe"
  Delete "$INSTDIR\examples\hello.de"
  RMDir "$INSTDIR\examples"
  Delete "$INSTDIR\docs\README.txt"
  RMDir "$INSTDIR\docs"
  Delete "$INSTDIR\uninstall.exe"
  RMDir "$INSTDIR"
  
  RMDir /r "$SMPROGRAMS\Defacto"
  
  Delete "$DESKTOP\Defacto Compiler.lnk"
  
  DeleteRegKey HKCU "Software\Defacto"
  DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Defacto"
  
  Push "$INSTDIR"
  Call un.RemoveFromPath
SectionEnd

; Functions
Function AddToPath
  Exch $0
  Push $1
  Push $2
  Push $3
  Push $4
  
  ReadRegStr $1 HKCU "Environment" "PATH"
  StrCmp $1 "" 0 +4
    WriteRegExpandStr HKCU "Environment" "PATH" "$0"
    Goto done
  
  StrCpy $2 $1 1
  StrCmp $2 ";" 0 +2
    StrCpy $1 $1 "" 1
  
  StrCpy $3 $1 "" -1
  StrCmp $3 ";" 0 +2
    StrCpy $1 $1 -1
  
  StrCpy $4 ";$0"
  StrCpy $2 "$1$4"
  
  WriteRegExpandStr HKCU "Environment" "PATH" "$2"
  
  done:
  Pop $4
  Pop $3
  Pop $2
  Pop $1
  Pop $0
  
  SendMessage ${HWND_BROADCAST} ${WM_SETTINGCHANGE} 0 "STR:Environment" /TIMEOUT=5000
FunctionEnd

Function un.RemoveFromPath
  Exch $0
  Push $1
  Push $2
  Push $3
  Push $4
  Push $5
  
  ReadRegStr $1 HKCU "Environment" "PATH"
  StrCmp $1 "" done
  
  StrCpy $2 "$0;"
  StrCpy $3 $1 1
  StrCmp $3 ";" 0 +2
    StrCpy $1 $1 "" 1
  
  StrCpy $4 ""
  
  loop:
    StrCpy $5 $1 1
    StrCmp $5 "" done2
    StrCmp $5 ";" 0 +5
      StrCpy $3 $4
      StrCpy $4 ""
      StrCmp $3 "$2" found
      StrCpy $4 "$3;"
      Goto next
    StrCpy $4 "$4$5"
    
    next:
    StrCpy $1 $1 "" 1
    Goto loop
  
  found:
  StrCpy $1 $4
  StrCpy $3 $1 "" -1
  StrCmp $3 ";" 0 +2
    StrCpy $1 $1 -1
  
  WriteRegExpandStr HKCU "Environment" "PATH" "$1"
  
  done2:
  Pop $5
  Pop $4
  Pop $3
  Pop $2
  Pop $1
  Pop $0
  
  SendMessage ${HWND_BROADCAST} ${WM_SETTINGCHANGE} 0 "STR:Environment" /TIMEOUT=5000
  
  done:
FunctionEnd

; Registry entries
Function .onInstSuccess
  WriteRegStr HKCU "Software\Defacto" "InstallDir" "$INSTDIR"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Defacto" "DisplayName" "Defacto Compiler v0.25"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Defacto" "UninstallString" "$INSTDIR\uninstall.exe"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Defacto" "InstallLocation" "$INSTDIR"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Defacto" "DisplayIcon" "$INSTDIR\defacto.exe"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Defacto" "Publisher" "Defacto Project"
FunctionEnd

Function .onInit
  ; Check if NASM is installed
  ReadRegStr $0 HKLM "SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\NASM" "DisplayIcon"
  StrCmp $0 "" 0 +2
    MessageBox MB_YESNO "NASM is not detected. It is required for assembling code.$\n$\nInstall NASM from https://www.nasm.us/ or use: choco install nasm$\n$\nContinue anyway?" IDNO abort
    Goto done
  abort:
    Abort
  done:
FunctionEnd
NSIS_EOF

makensis -V2 installer.nsi

if [ ! -f "defacto-0.25-installer.exe" ]; then
    echo "❌ Failed to build installer"
    exit 1
fi

mv "defacto-0.25-installer.exe" "../$OUTPUT"
cd ..

# Cleanup
rm -rf "$INSTALLER_PKG"

echo ""
echo "✅ Success! Created: $OUTPUT"
echo "📦 Size: $(du -h "$OUTPUT" | cut -f1)"
echo ""
echo "To test: wine $OUTPUT"
