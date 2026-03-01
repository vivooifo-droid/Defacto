; Defacto Compiler Windows Installer
; NSIS Script - Version 0.53

!include "MUI2.nsh"
!include "FileFunc.nsh"
!include "x64.nsh"

; General
Name "Defacto Compiler v0.53"
OutFile "defacto-0.53-installer.exe"
InstallDir "$PROGRAMFILES\Defacto"
InstallDirRegKey HKLM "Software\Defacto" "InstallDir"
RequestExecutionLevel admin

; Modern UI Settings
!define MUI_ABORTWARNING
!define MUI_ICON "installer_icon.ico"
!define MUI_UNICON "installer_icon.ico"
!define MUI_WELCOMEPAGE_TITLE "Welcome to the Defacto Compiler v0.53 Setup Wizard"
!define MUI_WELCOMEPAGE_TEXT "This wizard will guide you through the installation of Defacto Compiler v0.53 (alpha).\r\n\r\nDefacto is a low-level programming language for x86-32/64, ARM64, and bare-metal development.\r\n\r\n$(^Branding)"
!define MUI_FINISHPAGE_TITLE "Installation Complete"
!define MUI_FINISHPAGE_TEXT "Defacto Compiler v0.53 has been successfully installed.\r\n\r\nYou can now start using Defacto by opening a command prompt and typing 'defacto -h'.\r\n\r\nVisit the documentation at: https://github.com/vivooifo-droid/Defacto\r\n\r\n$(^Branding)"

; Pages
!insertmacro MUI_PAGE_WELCOME
!insertmacro MUI_PAGE_LICENSE "LICENSE"
!insertmacro MUI_PAGE_COMPONENTS
!insertmacro MUI_PAGE_DIRECTORY
!insertmacro MUI_PAGE_INSTFILES
!insertmacro MUI_PAGE_FINISH

!insertmacro MUI_UNPAGE_CONFIRM
!insertmacro MUI_UNPAGE_INSTFILES
!insertmacro MUI_UNPAGE_FINISH

; Components
!insertmacro MUI_COMPONENTSPAGE_NODESC

; Language
!insertmacro MUI_LANGUAGE "English"

; Installer Sections
Section "Defacto Compiler (Core)" SecCore
    SectionIn RO
    
    SetOutPath "$INSTDIR"
    
    ; Copy compiler binary
    File "compiler\defacto.exe"
    File "compiler\defacto"
    
    ; Copy documentation
    File "LICENSE"
    File "README.md"
    File "SYNTAX.md"
    File "INSTALL.md"
    
    ; Copy release notes
    File "RELEASE-v0.53.md"
    
    ; Create examples directory
    SetOutPath "$INSTDIR\examples"
    File "hello.de"
    File "example-driver.de"
    File "demo-v0.44.de"
    
    ; Create stdlib directory
    SetOutPath "$INSTDIR\stdlib"
    File /r "stdlib\*.*"
    
    ; Create tools directory
    SetOutPath "$INSTDIR\tools"
    File "tools\defo.sh"
    File "tools\deformat.sh"
    File "tools\delint.sh"
    
    ; Set output back to install dir
    SetOutPath "$INSTDIR"
    
    ; Create start menu shortcuts
    CreateDirectory "$SMPROGRAMS\Defacto"
    CreateShortcut "$SMPROGRAMS\Defacto\Defacto Documentation.lnk" "$INSTDIR\README.md"
    CreateShortcut "$SMPROGRAMS\Defacto\Defacto Syntax.lnk" "$INSTDIR\SYNTAX.md"
    CreateShortcut "$SMPROGRAMS\Defacto\Defacto Examples.lnk" "$INSTDIR\examples"
    CreateShortcut "$SMPROGRAMS\Defacto\Uninstall.lnk" "$INSTDIR\uninst.exe"
    
    ; Create desktop shortcut
    CreateShortcut "$DESKTOP\Defacto.lnk" "$INSTDIR\defacto.exe"
    
    ; Add to PATH (using proper method)
    Push "$INSTDIR"
    Call AddToPath
    
    ; Write registry
    WriteRegStr HKLM "Software\Defacto" "InstallDir" "$INSTDIR"
    WriteRegStr HKLM "Software\Defacto" "Version" "0.53.0"
    WriteRegDWORD HKLM "Software\Defacto" "VersionMajor" 0
    WriteRegDWORD HKLM "Software\Defacto" "VersionMinor" 53
    
    ; Write uninstaller
    WriteUninstaller "$INSTDIR\uninst.exe"
SectionEnd

Section "Visual Studio Code Extension" SecVSCode
    ; Note: This would require VSCode to be installed
    ; For now, just create a shortcut to the extension directory
    SetOutPath "$INSTDIR\vscode-extension"
    File /r "vscode-extension\*.*"
    
    CreateShortcut "$SMPROGRAMS\Defacto\Install VSCode Extension.lnk" "$INSTDIR\vscode-extension\install.bat"
SectionEnd

Section "C++ Addons" SecCppAddons
    SetOutPath "$INSTDIR\addons\cpp"
    File /r "addons\cpp\*.*"
SectionEnd

Section "Sample Projects" SecSamples
    SetOutPath "$INSTDIR\samples"
    File /r "examples\*.*"
SectionEnd

; Uninstall Section
Section "Uninstall"
    ; Remove from PATH
    Push "$INSTDIR"
    Call RemoveFromPath
    
    ; Remove registry
    DeleteRegKey /ifempty HKLM "Software\Defacto"
    
    ; Remove files
    RMDir /r "$INSTDIR"
    
    ; Remove start menu
    RMDir /r "$SMPROGRAMS\Defacto"
    
    ; Remove desktop shortcut
    Delete "$DESKTOP\Defacto.lnk"
    
    ; Remove uninstaller
    Delete "$INSTDIR\uninst.exe"
SectionEnd

; Functions

Function .onInit
    ReadRegStr $0 HKLM "Software\Defacto" "InstallDir"
    StrCmp $0 "" done
    StrCpy $INSTDIR $0
done:
FunctionEnd

Function AddToPath
    Exch $0
    Push $1
    Push $2
    
    ; Read current PATH
    ReadRegStr $1 HKLM "SYSTEM\CurrentControlSet\Control\Session Manager\Environment" "PATH"
    
    ; Check if already in PATH
    StrCpy $2 $1
    Push $0
    Call StrContains
    Pop $2
    StrCmp $2 "" add
    Goto done
    
add:
    ; Add to PATH
    StrCpy $1 "$1;$0"
    WriteRegExpandStr HKLM "SYSTEM\CurrentControlSet\Control\Session Manager\Environment" "PATH" "$1"
    
    ; Broadcast WM_SETTINGCHANGE
    System::Call 'user32::SendMessageA(i, i, i, i) i ?i' \
        ${HWND_BROADCAST} \
        ${WM_SETTINGCHANGE} \
        0 \
        "rEnvironment"
    
done:
    Pop $2
    Pop $1
    Pop $0
FunctionEnd

Function RemoveFromPath
    Exch $0
    Push $1
    Push $2
    Push $3
    Push $4
    
    ; Read current PATH
    ReadRegStr $1 HKLM "SYSTEM\CurrentControlSet\Control\Session Manager\Environment" "PATH"
    
    ; Remove from PATH
    StrCpy $2 $1
    Push $0
    Call StrContains
    Pop $2
    StrCmp $2 "" done
    
    ; Remove the path
    Push "$1"
    Push "$0"
    Call UnStrStr
    Pop $3
    
    WriteRegExpandStr HKLM "SYSTEM\CurrentControlSet\Control\Session Manager\Environment" "PATH" "$3"
    
    ; Broadcast WM_SETTINGCHANGE
    System::Call 'user32::SendMessageA(i, i, i, i) i ?i' \
        ${HWND_BROADCAST} \
        ${WM_SETTINGCHANGE} \
        0 \
        "rEnvironment"
    
done:
    Pop $4
    Pop $3
    Pop $2
    Pop $1
    Pop $0
FunctionEnd

; String contains function
Function StrContains
    Exch $0
    Exch
    Exch $1
    Push $2
    Push $3
    Push $4
    Push $5
    
    StrCpy $2 0
    StrLen $3 $0
    StrLen $4 $1
    
    loop:
        IntCmp $2 $4 done
        StrCpy $5 $1 $3 $2
        StrCmp $5 $0 found
        IntOp $2 $2 + 1
        Goto loop
    
    found:
        Pop $5
        Pop $4
        Pop $3
        Exch $0
        Return
    
    done:
        StrCpy $0 ""
        Pop $5
        Pop $4
        Pop $3
        Exch $0
FunctionEnd

; UnStrStr - Remove substring from string
Function UnStrStr
    Exch $1 ; substring to remove
    Exch
    Exch $0 ; original string
    Push $2
    Push $3
    Push $4
    Push $5
    Push $6
    
    StrCpy $2 "" ; result
    StrCpy $3 0 ; position
    StrLen $4 $1 ; length of substring
    StrLen $5 $0 ; length of original
    
    loop:
        IntCmp $3 $5 done_unstr
        StrCpy $6 $0 $4 $3
        StrCmp $6 $1 skip
        StrCpy $6 $0 1 $3
        StrCpy $2 "$2$6"
        IntOp $3 $3 + 1
        Goto loop
        
    skip:
        IntOp $3 $3 + $4
        Goto loop
        
    done_unstr:
        Pop $6
        Pop $5
        Pop $4
        Pop $3
        Exch $2
FunctionEnd

; Constants for SendMessage
!define HWND_BROADCAST 0xffff
!define WM_SETTINGCHANGE 0x001A
