# Defacto Windows Installer Guide

## Быстрый способ (рекомендуется)

### 1. На macOS/Linux (кросс-компиляция)

```bash
python3 build-installer.py
```

Это создаст:
- `compiler/defacto.exe` — скомпилированный компилятор
- `installer_pkg/` — структура установщика

### 2. На Windows (финальная сборка)

Нужно установить **NSIS** (один раз):
- Скачать: https://nsis.sourceforge.io/download/
- Установить (Next → Next → Finish)

Затем открыть командную строку:
```cmd
cd path\to\defacto
makensis installer_pkg\installer.nsi
```

Результат: `installer_pkg\defacto-0.25-installer.exe`

---

## Что находится в установщике

✅ `defacto.exe` — компилятор  
✅ Примеры программ (`examples/hello.de`)  
✅ Документация (`docs/README.txt`)  
✅ Start Menu ярлыки  
✅ Добавление в PATH автоматически  
✅ Встроенный деинсталлятор  

---

## Требования

### На компьютере разработчика (macOS/Linux):
```bash
brew install mingw-w64     # macOS
# или
sudo apt install mingw-w64 # Linux
```

### На Windows (для конечного пользователя):
- NASM: https://www.nasm.us/ (или `choco install nasm`)

---

## Альтернативный способ (вручную)

Если Python скрипт не подходит:

1. Собрать компилятор:
```bash
cd compiler
make windows
cd ..
```

2. Создать `installer_pkg/`:
```cmd
mkdir installer_pkg
copy compiler\defacto.exe installer_pkg\
copy hello.de installer_pkg\
copy README.md installer_pkg\README.txt
```

3. Собрать установщик:
```cmd
makensis installer_pkg\installer.nsi
```

---

## Файлы установщика

- `installer.nsi` — NSIS скрипт
- `build-installer.py` — Python скрипт для автоматизации
- `installer_pkg/` — подготовленные файлы

---

## Распространение

Готовый файл `defacto-0.25-installer.exe` можно распространять как обычный Windows установщик!

Пользователь просто:
1. Скачивает `.exe`
2. Запускает
3. Нажимает "Next" несколько раз
4. Готово!
