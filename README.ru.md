# Defacto v0.30 (pre-alpha)

Низкоуровневый язык для x86-32, bare-metal экспериментов и собственных тулчейнов.
Статус pre-alpha: язык и тулчейн нестабильны и будут меняться.

English version: `README.md`

## Содержимое репозитория

- Компилятор (`compiler/`)
- VS Code расширение (`vscode-extension/`)
- Пакетный менеджер `defo`
- C++ аддоны (`addons/cpp/`)
- **Rust аддоны** (`addons/rust/`) — пишите библиотеки на Rust!
- **Backend фреймворк** (`addons/rust-backend/`) — HTTP веб-фреймворк для Defacto

## Установка

### Windows (Рекомендуется)

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

```
xcode-select --install
brew install nasm mingw-w64
cd compiler && make
./defacto -h
```

### Linux (Ubuntu/Debian)

```
sudo apt update
sudo apt install build-essential nasm
cd compiler && make
./defacto -h
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
p.x = 100        // поле структуры
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

// Использование драйвера
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
// x и msg автоматически освобождаются здесь
```

`free{}` по-прежнему поддерживается для обратной совместимости.

## Комментарии и строки

- Комментарии строки: `//`.
- Строки поддерживают `\n` и `\t`.

```de
// Это комментарий
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

### Rust аддоны (Рекомендуется)

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
