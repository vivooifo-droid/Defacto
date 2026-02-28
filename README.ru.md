# Defacto v0.35 (pre-alpha)

Низкоуровневый язык для x86-32, bare-metal экспериментов и собственных тулчейнов.
Статус pre-alpha: язык и тулчейн нестабильны и будут меняться.

**Что нового в v0.35:**
- **Указатели** — синтаксис как в Rust (`*i32`, `&x`, `*ptr`, `ptr->field`)
- **Системный аллокатор** — поддержка malloc/free в terminal режиме
- **Null указатели** — `var ptr: *i32 = null`
- **Двойные указатели** — `var ptr: **i32`
- **Авто-режим для macOS** — по умолчанию используется `-terminal-macos`

English version: `README.md`

## Содержимое репозитория

- Компилятор (`compiler/`)
- VS Code расширение (`vscode-extension/`)
- Пакетный менеджер `defo`
- C++ аддоны (`addons/cpp/`)
- **Rust аддоны** (`addons/rust/`) — пишите библиотеки на Rust!
- **Backend фреймворк** (`addons/rust-backend/`) — HTTP веб-фреймворк для Defacto

## Установка

### Windows

Скачайте и запустите установщик: [defacto-0.30-installer.exe](https://github.com/vivooifo-droid/Defacto/releases/download/v0.30/defacto-0.30-installer.exe)

Установщик:
- Установит компилятор Defacto в `C:\Program Files\Defacto`
- Добавит Defacto в PATH
- Создаст ярлыки в меню Пуск
- Включит примеры файлов

**Требования:** Сначала установите NASM:
- Скачать: https://www.nasm.us/
- Или через Chocolatey: `choco install nasm`

### macOS

**Требования:**
- Xcode Command Line Tools
- NASM assembler

**Примечание:** macOS по умолчанию использует `-terminal-macos`, создавая нативные x86_64 бинарники.
На Apple Silicon (M1/M2/M3) для запуска требуется Rosetta 2.

```bash
# Установить Xcode tools
xcode-select --install

# Установить NASM
brew install nasm

# Собрать компилятор
cd compiler && make

# Проверить установку
./defacto -h

# Установить Rosetta (только Apple Silicon)
softwareupdate --install-rosetta
```

### Linux (Ubuntu/Debian)

**Требования:**
- Build essentials (gcc, make)
- NASM assembler

**Примечание:** Linux по умолчанию использует `-terminal`, создавая ELF 32-битные бинарники с libc.

```bash
# Установить зависимости
sudo apt update
sudo apt install build-essential nasm

# Собрать компилятор
cd compiler && make

# Проверить установку
./defacto -h
```

**Другие дистрибутивы Linux:**

```bash
# Fedora
sudo dnf install gcc make nasm

# Arch Linux
sudo pacman -S gcc make nasm
```

## Сборка из исходников

Смотрите инструкции установки выше для вашей платформы, или:

```bash
cd compiler
make
./defacto -h
```

## Использование

### Быстрый старт

**macOS** (по умолчанию `-terminal-macos`):
```bash
./defacto hello.de -o hello
./hello
```

**Linux** (по умолчанию `-terminal`):
```bash
./defacto hello.de -o hello
./hello
```

**Bare-metal ядро** (все платформы):
```bash
./defacto -kernel os.de -o kernel.bin
```

### Командная строка

Компиляция программы:
```bash
./defacto program.de
```

Указать выходной файл:
```bash
./defacto program.de -o output
./defacto -o output program.de    # порядок не важен
```

Только ассемблер:
```bash
./defacto -S program.de
```

Подробный режим:
```bash
./defacto -v program.de
```

Помощь:
```bash
./defacto -h
```

## Режимы

| Режим | Платформа | Вывод | По умолчанию |
|-------|-----------|-------|--------------|
| `-kernel` | Все | Binary (x86-32) | Linux |
| `-terminal` | Linux | ELF 32-bit | Linux |
| `-terminal-macos` | macOS | Mach-O 64-bit | macOS |

**Выбор режима:**
- **macOS**: По умолчанию `-terminal-macos` (нативные macOS бинарники)
- **Linux**: По умолчанию `-terminal` (Linux syscalls)
- **Bare-metal**: Используйте `-kernel` для разработки ОС (без зависимостей)

### Примеры

```bash
# macOS приложение
./defacto app.de -o app
./app

# Linux приложение (на Linux)
./defacto app.de -o app
./app

# Bare-metal ядро
./defacto -kernel kernel.de -o kernel.bin

# Запуск в QEMU
qemu-system-i386 -kernel kernel.bin
```

## Полный синтаксис языка

### Обязательные директивы

```de
#Mainprogramm.start
#NO_RUNTIME
#SAFE

#Mainprogramm.end
```

Примечания:

- `#Mainprogramm.start` / `#Mainprogramm.end` обязательны.
- `#NO_RUNTIME` обязателен для bare-metal.
- `#SAFE` зарезервирован; проверки памяти всегда включены.
- `#INTERRUPT {num} == func` парсится, но не генерируется.

### Секции

Код должен быть в секциях:

```de
<.de
    var x: i32 = 0
    var msg: string = "hello"

    static.pl>

    display{msg}
    x = (x + 1)
.>
```

Правила:

- До `static.pl>` только `var` и `const`.
- После `static.pl>` только инструкции.
- `.>` закрывает секцию.

### Типы и переменные

```de
var count: i32 = 42
const big: i64 = 1000000
```

Поддерживаемые типы:

| Тип | Размер | Описание |
| --- | --- | --- |
| `i32` | 4 байта | знаковое целое |
| `i64` | 8 байт | знаковое целое |
| `u8` | 1 байт | беззнаковый байт |
| `string` | указатель | null-терминированная строка |
| `pointer` | 4 байта | сырой адрес |
| `struct_name` | varies | пользовательский тип структуры |

Массивы:

```de
var buf: u8[64]
var arr: i32[10]
```

Правила:

- `const` массивы не поддерживаются.
- `const` должны быть инициализированы.

### Структуры (v0.30+)

Определение пользовательских структур данных:

```de
struct Point {
    x: i32
    y: i32
    z: i32
}

var p: Point
p.x = 10
p.y = 20
p.z = 30
```

Правила:

- Поля структур могут быть любого встроенного типа.
- Структуры хранятся inline (без указателей).
- Доступ к полям через точку: `struct.field`

### Выражения

Поддерживаются только простые выражения:

```de
x = (a + b)
x = (x - 1)
x = (x * 2)
x = (x / 4)
```

Ограничения:

- Только один оператор.
- Нет вложенных выражений `(a + b + c)`.
- Отрицательные литералы не поддерживаются. Используйте `(0 - 1)` или временную переменную.
- Индексы массивов в выражениях — только число/переменная/регистр.

### Инструкции

#### Присваивание

```de
x = 1
x = other
x = (x + 1)
arr[i] = x
p.x = 100
```

#### Цикл

```de
loop {
    if x == 10 { stop }
}
```

#### If/Else (v0.30+)

```de
if x == y {
    display{msg}
} else {
    display{err}
}
```

Правила:

- Поддерживается только `==` для сравнения.
- Слева и справа один токен: число, переменная или регистр.
- Блок `else` необязателен.

### Функции

```de
function == my_func {
    <.de
        var a: i32 = 0
        static.pl>
        a = (a + 1)
    .>
}

call #my_func
```

### Драйверы

Определение и использование аппаратных драйверов:

```de
<drv.
Const.driver = keyboard_driver
keyboard_driver <<func = keyboard>>
.dr>

call #keyboard_driver
```

Поддерживаемые типы драйверов: `keyboard`, `mouse`, `volume`

### Встроенные функции

```de
display{msg}
color{10}
readkey{key}
readchar{ch}
putchar{65}
clear{}
reboot{}
```

Примечания:

- `display{string}` печатает строку. В terminal режиме добавляет перевод строки.
- `readkey` и `readchar` работают только в bare-metal. В terminal возвращают `0`.
- `readchar` поддерживает ASCII `a-z`, `0-9`, пробел, enter, backspace.
- `color`, `putchar`, `clear`, `reboot` работают только в bare-metal.

### Регистры

```de
#MOV {#R1, x}
#MOV {x, #R1}
#R1 = (#R1 + #R2)
```

Примечания:

- Привязка регистров фиксирована к x86-32.
- `#STATIC` парсится, но не генерируется.

## Управление памятью

**Автоматическое управление памятью (v0.30+):**

Компилятор автоматически освобождает все переменные в конце каждой секции. Вам больше не нужно вызывать `free{}` вручную.

```de
<.de
    var x: i32 = 0
    var msg: string = "hello"
    static.pl>
    display{msg}
.>
```

`free{}` по-прежнему поддерживается для обратной совместимости.

## Комментарии и строки

- Комментарии строки: `//`.
- Строки поддерживают `\n` и `\t`.

```de
var msg: string = "Привет\nМир"
```

## Пакетный менеджер defo

```
./defo init my-app
./defo install https://github.com/owner/repo.git
./defo install owner/repo
./defo install owner/repo#main
./defo list
```

## Ограничения

- Только `==` в условиях `if`.
- Один оператор на выражение.
- Нет отрицательных числовых литералов.
- Нельзя использовать выражения в индексах массивов.
- Interrupt директивы парсятся, но не генерируются.

## Аддоны

Расширяйте Defacto нативными библиотеками!

### Rust аддоны

Пишите высокопроизводительные аддоны на Rust с безопасностью памяти:

```bash
cd addons/rust
cargo build --release
```

Полная документация: [`addons/rust/README.md`](addons/rust/README.md)

### Backend фреймворк (Rust)

HTTP веб-фреймворк для создания серверов:

```bash
cd addons/rust-backend
cargo build --release
```

Документация: [`addons/rust-backend/README.md`](addons/rust-backend/README.md)

### C++ аддоны

Традиционные C++ аддоны также поддерживаются:

```bash
cd addons/cpp
make
```

Примеры: [`addons/cpp/`](addons/cpp/)

## Что нового в v0.30

- **Автоматическое управление памятью** - Больше не нужен `free{}`!
- **if/else** - Полная поддержка условных переходов
- **Структуры** - Определение пользовательских структур данных
- **Доступ к полям** - Доступ к полям структур через точку
- **Улучшенный установщик Windows** - Упрощённый процесс установки
