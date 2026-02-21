# Defacto (pre-alpha)

Низкоуровневый язык для x86-32, bare-metal экспериментов и собственных тулчейнов.
Статус pre-alpha: язык и тулчейн нестабильны и будут меняться.

English version: `README.md`

## Содержимое репозитория

- Компилятор (`compiler/`)
- VS Code расширение (`vscode-extension/`)
- Наивный пакетный менеджер `defo`
- Песочница C++ аддонов (`addons/`)

## Установка

### Windows (Рекомендуется)

Скачайте и запустите установщик: [defacto-0.25-installer.exe](https://github.com/vivooifo-droid/Defacto/releases/download/v0.25/defacto-0.25-installer.exe)

Установщик:
- Установит компилятор Defacto в `C:\Program Files\Defacto`
- Добавит Defacto в PATH
- Создаст ярлыки в меню Пуск
- Включит примеры файлов

**Требования:** Сначала установите NASM:
- Скачать: https://www.nasm.us/
- Или через Chocolatey: `choco install nasm`

### Windows (Сборка из исходников через MSYS2)

1. Установить MSYS2.
2. Открыть **MSYS2 MINGW64** терминал.
3. Установить инструменты:

```
pacman -S --needed mingw-w64-x86_64-toolchain make nasm git
```

### macOS

```
xcode-select --install
brew install nasm mingw-w64 nsis
./build-windows-installer.sh
```

### Linux (Ubuntu/Debian)

```
sudo apt update
sudo apt install build-essential nasm nsis mingw-w64
./build-windows-installer.sh
```

## Сборка компилятора

```
cd compiler
make
./defacto -h
```

## Использование

Компиляция программы:

```
./defacto program.de
```

Выходной файл:

```
./defacto -o output.bin program.de
```

Только ассемблер:

```
./defacto -S program.de
```

Подробный вывод:

```
./defacto -v program.de
```

## Режимы

- `-kernel` по умолчанию bare-metal (x86-32)
- `-terminal` Linux syscalls
- `-terminal-macos` macOS syscalls (Mach-O x86_64)

## Полный синтаксис (кратко)

### Обязательные директивы

```
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

```
<.de
    var x: i32 = 0
    var msg: string = "hello"

    static.pl>

    display{msg}
    x = (x + 1)
    free{x}
    free{msg}
.>
```

Правила:

- До `static.pl>` только `var` и `const`.
- После `static.pl>` только инструкции.
- `.>` закрывает секцию.

### Типы и переменные

```
var count: i32 = 42
const big: i64 = 1000000
```

Поддерживаемые типы:

| Type | Size | Notes |
| --- | --- | --- |
| `i32` | 4 bytes | signed integer |
| `i64` | 8 bytes | signed integer |
| `u8` | 1 byte | unsigned byte |
| `string` | pointer | null-terminated string |
| `pointer` | 4 bytes | raw address |

Массивы:

```
var buf: u8[64]
var arr: i32[10]
```

Правила:

- `const` массивы не поддерживаются.
- `const` должны быть инициализированы.

### Выражения

Поддерживаются только простые выражения:

```
x = (a + b)
x = (x - 1)
x = (x * 2)
x = (x / 4)
```

Ограничения:

- Только один оператор.
- Нет вложенных выражений `(a + b + c)`.
- Отрицательные литералы не поддерживаются. Используй `(0 - 1)` или временную переменную.
- Индексы массивов в выражениях — только число/переменная/регистр.

### Инструкции

Присваивание:

```
x = 1
x = other
x = (x + 1)
arr[i] = x
```

Цикл:

```
loop {
    if x == 10 { stop }
}
```

Условие:

```
if x == y { stop }
```

Правила:

- Поддерживается только `==`.
- Слева и справа один токен: число, переменная или регистр.
- Нет else/elseif.

### Функции

```
function == my_func {
    <.de
        var a: i32 = 0
        static.pl>
        a = (a + 1)
        free{a}
    .>
}

call #my_func
```

### Встроенные

```
display{msg}
free{x}
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

```
#MOV {#R1, x}
#MOV {x, #R1}
#R1 = (#R1 + #R2)
```

Примечания:

- Привязка регистров фиксирована к x86-32.
- `#STATIC` парсится, но не генерируется.

## Правила памяти

- Каждая `var` должна освобождаться через `free{}`.
- `const` освобождать нельзя.
- Строки и массивы, объявленные как `var`, должны быть освобождены.

## Комментарии и строки

- Комментарии строки: `//`.
- Строки поддерживают `\n` и `\t`.

## Пакетный менеджер defo

```
./defo init my-app
./defo install https://github.com/owner/repo.git
./defo install owner/repo
./defo install owner/repo#main
./defo list
```

## Ограничения

- Только `==` в `if`.
- Нет else/elseif.
- Один оператор на выражение.
- Нет отрицательных числовых литералов.
- Нельзя использовать выражения в индексах массивов.
- Interrupt директивы парсятся, но не генерируются.
