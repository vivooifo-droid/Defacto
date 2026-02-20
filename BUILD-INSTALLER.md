# How to Build defacto-installer.exe

## On Windows (One-time setup)

### 1. Install PyInstaller
```powershell
pip install pyinstaller
```

### 2. Build the installer
```powershell
cd path\to\Defacto
pyinstaller --onefile defacto-installer-win.py
```

### 3. Get the .exe
```
dist\defacto-installer.exe
```

---

## For Distribution

1. Compile with PyInstaller (see above)
2. Upload `defacto-installer.exe` to GitHub Releases
3. Share the download link with users

---

## Optional: Add Icon

If you have an icon file (`icon.ico`):
```powershell
pyinstaller --onefile --icon=icon.ico defacto-installer-win.py
```

---

## What the installer does

✅ Beautiful GUI window  
✅ Checks dependencies (git, g++, make)  
✅ Downloads from GitHub  
✅ Builds compiler  
✅ Installs to Program Files  
✅ Adds to PATH  
✅ Shows success message  

---

## Usage

Just double-click `defacto-installer.exe` and follow the prompts!

No terminal needed. 🎉
