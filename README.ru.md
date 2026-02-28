# Defacto v0.45 (alpha)

Низкоуровневый язык для x86-32, bare-metal экспериментов и собственных тулчейнов.

**Что нового в v0.45:**
- Чистый синтаксис — старый синтаксис удалён
- Обратная совместимость не поддерживается — только новый синтаксис fn, driver, for..to
- Упрощённая кодовая база — удалён легаси код парсера

English version: `README.md`

## Содержимое репозитория

- Компилятор (`compiler/`)
- VS Code расширение (`vscode-extension/`)
- Пакетный менеджер `defo`
- Rust аддоны (`addons/rust/`)
- Backend фреймворк (`addons/rust-backend/`)

## Установка

### macOS

```bash
# Установить Xcode tools
xcode-select --install

# Установить NASM
brew install nasm

# Собрать компилятор
cd compiler && make

# Проверить установку
./defacto -h
```

### Linux (Ubuntu/Debian)

```bash
# Установить зависимости
sudo apt update
sudo apt install build-essential nasm

# Собрать компилятор
cd compiler && make

# Проверить установку
./defacto -h
```

## Использование

```bash
# Компилировать
./defacto hello.de -o hello

# Запустить
./hello
```

### Режимы

| Режим | Платформа | Вывод |
|------|----------|-------|
| `-kernel` | Все | Binary (x86-32) |
| `-terminal` | Linux | ELF 32-bit |
| `-terminal-macos` | macOS | Mach-O 64-bit |

## Синтаксис

### Структура файла

```de
#Mainprogramm.start
#NO_RUNTIME
#SAFE

fn main {
    <.de
        var msg: string = "Hello"
        display{msg}
    .>
}

#Mainprogramm.end
```

### Переменные

```de
var x: i32 = 42
const MAX: i32 = 100
```

### Функции

```de
fn add(a: i32, b: i32) {
    <.de
        var result: i32 = 0
        result = (a + b)
    .>
}

call #add
```

### Цикл for

```de
for i = 0 to 10 {
    display{i}
}
```

### Драйверы

```de
driver keyboard {
    type = keyboard
}

call #keyboard
```

## Ограничения

- Нет индексов выражений в массивах: `arr[i + 1]` не поддерживается
- Директивы прерываний парсятся, но не генерируются

## Документация

- [SYNTAX.md](SYNTAX.md) — полная документация по синтаксису
- [RELEASE-v0.45.md](RELEASE-v0.45.md) — заметки о релизе
