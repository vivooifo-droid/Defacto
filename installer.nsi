; Defacto Compiler Windows Installer
; NSIS Script - Version 0.40

!include "MUI2.nsh"
!include "FileFunc.nsh"

; General
Name "Defacto Compiler v0.40"
OutFile "defacto-0.40-installer.exe"
InstallDir "$PROGRAMFILES\Defacto"
InstallDirRegKey HKLM "Software\Defacto" "InstallDir"
RequestExecutionLevel admin

; Modern UI Settings
!define MUI_ABORTWARNING
!define MUI_ICON "installer_icon.ico"
!define MUI_UNICON "installer_icon.ico"

; Pages
!insertmacro MUI_PAGE_WELCOME
!insertmacro MUI_PAGE_LICENSE "LICENSE"
!insertmacro MUI_PAGE_DIRECTORY
!insertmacro MUI_PAGE_INSTFILES
!insertmacro MUI_PAGE_FINISH

!insertmacro MUI_UNPAGE_CONFIRM
!insertmacro MUI_UNPAGE_INSTFILES
!insertmacro MUI_UNPAGE_FINISH

; Language
!insertmacro MUI_LANGUAGE "English"

; Installer Sections
Section "Defacto Compiler" SecDefacto
    SetOutPath "$INSTDIR"
    
    ; Copy compiler files
    File "compiler\defacto.exe"
    File "compiler\defacto"
    File "LICENSE"
    File "README.md"
    
    ; Copy example files
    CreateDirectory "$INSTDIR\examples"
    File "hello.de"
    File "example-driver.de"
    
    ; Create start menu shortcuts
    CreateDirectory "$SMPROGRAMS\Defacto"
    CreateShortcut "$SMPROGRAMS\Defacto\Defacto Documentation.lnk" "$INSTDIR\README.md"
    CreateShortcut "$SMPROGRAMS\Defacto\Defacto Examples.lnk" "$INSTDIR\examples"
    CreateShortcut "$SMPROGRAMS\Defacto\Uninstall.lnk" "$INSTDIR\uninst.exe"
    
    ; Add to PATH
    nsExec::Exec 'setx PATH "%PATH%;$INSTDIR" /m'
    
    ; Write registry
    WriteRegStr HKLM "Software\Defacto" "InstallDir" "$INSTDIR"
    WriteRegStr HKLM "Software\Defacto" "Version" "0.30.0"
    
    ; Write uninstaller
    WriteUninstaller "$INSTDIR\uninst.exe"
SectionEnd

Section "Create Desktop Shortcut"
    CreateShortcut "$DESKTOP\Defacto.lnk" "$INSTDIR\defacto.exe"
SectionEnd

; Uninstaller
Section "Uninstall"
    ; Remove from PATH (best effort)
    nsExec::Exec 'setx PATH "%PATH%;$INSTDIR" /m'
    
    ; Remove registry
    DeleteRegKey HKLM "Software\Defacto"
    
    ; Remove files
    RMDir /r "$INSTDIR"
    
    ; Remove start menu
    RMDir /r "$SMPROGRAMS\Defacto"
    
    ; Remove desktop shortcut
    Delete "$DESKTOP\Defacto.lnk"
SectionEnd

; Functions
Function .onInit
    ReadRegStr $0 HKLM "Software\Defacto" "InstallDir"
    StrCmp $0 "" done
    StrCpy $INSTDIR $0
done:
FunctionEnd
