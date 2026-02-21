; Defacto Compiler Windows Installer
; Built with NSIS

!include "MUI2.nsh"
!include "x64.nsh"

; General settings
Name "Defacto Compiler v0.25"
OutFile "defacto-0.25-installer.exe"
InstallDir "$PROGRAMFILES\Defacto"
InstallDirRegKey HKCU "Software\Defacto" "InstallDir"

; Request admin privileges
RequestExecutionLevel admin

; UI Settings
!insertmacro MUI_PAGE_DIRECTORY
!insertmacro MUI_PAGE_INSTFILES
!insertmacro MUI_PAGE_FINISH

!insertmacro MUI_LANGUAGE "English"
!insertmacro MUI_LANGUAGE "Russian"

; Installer sections
Section "Install"
  SetOutPath "$INSTDIR"
  
  ; Copy main executable
  File "compiler\defacto.exe"
  
  ; Create example files directory
  CreateDirectory "$INSTDIR\examples"
  File /oname=examples\hello.de "hello.de"
  
  ; Create documentation
  CreateDirectory "$INSTDIR\docs"
  File /oname=docs\README.txt "README.md"
  
  ; Add to PATH
  ${If} ${RunningX64}
    SetRegView 64
  ${EndIf}
  
  ReadRegStr $0 HKCU "Environment" "PATH"
  ${If} $0 == ""
    StrCpy $0 "$INSTDIR"
  ${Else}
    StrCpy $0 "$0;$INSTDIR"
  ${EndIf}
  WriteRegExpandStr HKCU "Environment" "PATH" $0
  
  ; Broadcast WM_SETTINGCHANGE for PATH update
  SendMessage ${HWND_BROADCAST} ${WM_SETTINGCHANGE} 0 "STR:Environment" /TIMEOUT=5000
  
  ; Create Start Menu shortcuts
  CreateDirectory "$SMPROGRAMS\Defacto"
  CreateShortCut "$SMPROGRAMS\Defacto\Defacto Compiler.lnk" "$INSTDIR\defacto.exe" "" "$INSTDIR\defacto.exe" 0
  CreateShortCut "$SMPROGRAMS\Defacto\Examples.lnk" "$INSTDIR\examples" "" "" 0
  CreateShortCut "$SMPROGRAMS\Defacto\Uninstall.lnk" "$INSTDIR\uninstall.exe" "" "$INSTDIR\uninstall.exe" 0
  
  ; Create uninstaller
  WriteUninstaller "$INSTDIR\uninstall.exe"
  
  ; Registry entries
  WriteRegStr HKCU "Software\Defacto" "InstallDir" "$INSTDIR"
  WriteRegStr HKCU "Software\Microsoft\Windows\CurrentVersion\Uninstall\Defacto" "DisplayName" "Defacto Compiler"
  WriteRegStr HKCU "Software\Microsoft\Windows\CurrentVersion\Uninstall\Defacto" "UninstallString" "$INSTDIR\uninstall.exe"
  WriteRegStr HKCU "Software\Microsoft\Windows\CurrentVersion\Uninstall\Defacto" "InstallLocation" "$INSTDIR"
  
  MessageBox MB_OK "Defacto Compiler installed successfully!$\n$\nMake sure NASM is installed:$\nhttps://www.nasm.us/$\n$\nOr use: choco install nasm"
SectionEnd

; Uninstaller
Section "Uninstall"
  Delete "$INSTDIR\defacto.exe"
  Delete "$INSTDIR\uninstall.exe"
  RMDir /r "$INSTDIR\examples"
  RMDir /r "$INSTDIR\docs"
  RMDir "$INSTDIR"
  
  RMDir /r "$SMPROGRAMS\Defacto"
  
  DeleteRegKey HKCU "Software\Defacto"
  DeleteRegKey HKCU "Software\Microsoft\Windows\CurrentVersion\Uninstall\Defacto"
SectionEnd

; On successful install
Function .onInstSuccess
  Exec "cmd.exe /c defacto -h"
FunctionEnd
