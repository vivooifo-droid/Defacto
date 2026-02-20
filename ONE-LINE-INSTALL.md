# Defacto - One-Line Installation

## Windows (PowerShell)

### Локально (если есть папка проекта):
```powershell
powershell -ExecutionPolicy Bypass -File quick-install.ps1
```

### Со скачиванием (одна команда):
```powershell
powershell -Command "iwr https://raw.githubusercontent.com/artemposehonov/defacto/main/quick-install.ps1 -UseBasicParsing | iex"
```

---

## macOS / Linux

### Локально:
```bash
python3 quick-install.py
```

### Со скачиванием (одна команда):
```bash
python3 -c "$(curl -fsSL https://raw.githubusercontent.com/artemposehonov/defacto/main/quick-install.py)"
```

---

## Что происходит:

1. ✅ Проверяет зависимости (g++, make)
2. ✅ Скачивает проект (если нужно)
3. ✅ Собирает компилятор
4. ✅ Устанавливает в PATH
5. ✅ Готово к использованию!

---

## Требования

**Windows:**
- Visual Studio Community или MinGW-w64
- Make: `choco install make`
- NASM: `choco install nasm`

**macOS:**
```bash
xcode-select --install
brew install make nasm
```

**Linux:**
```bash
sudo apt install build-essential nasm make
```

---

## Использование после установки

```bash
defacto -h                           # Help
defacto -terminal program.de         # Compile
defacto -kernel -o kernel.bin os.de  # Bare-metal
defacto -S program.de                # Assembly only
```

---

## Если что-то не работает

1. Убедитесь, что установлены все зависимости
2. Перезагрузитесь (Windows) или перезапустите терминал
3. Проверьте: `defacto -h`

Всё! 🚀
