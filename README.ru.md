# Defacto v0.46 (alpha)

Низкоуровневый язык программирования для x86-32, bare-metal экспериментов и собственных тулчейнов.

**Что нового в v0.46:**
- **Массивы в структурах** — `struct Buffer { data: u8[256] }`
- **Switch/case** — `switch x { case 1: ... default: ... }`
- **Extern функции** — `extern printf` для интеграции с C библиотеками
- **Include файлов** — `include "path.de"` для модуляризации кода
- **Compile-time константы** — const выражения вычисляются на этапе компиляции

## Содержимое репозитория

- Компилятор (`compiler/`)
- VS Code расширение (`vscode-extension/`)
- Пакетный менеджер `defo`
- C++ аддоны (`addons/cpp/`)
- **Rust аддоны** (`addons/rust/`) — пишите библиотеки на Rust!
- **Backend фреймворк** (`addons/rust-backend/`) — HTTP веб-фреймворк для Defacto

## Установка

### macOS (Homebrew) — Рекомендуется

**Самый простой способ установить Defacto:**

```bash
# Добавить tap Defacto
brew tap vivooifo-droid/Defacto

# Установить Defacto
brew install defacto

# Проверить установку
defacto -h
```

Автоматически:
- Загрузит и установит зависимость NASM
- Соберёт и установит компилятор
- Добавит `defacto` в PATH

**Для Mac на Apple Silicon:** Для запуска бинарников `-terminal-macos` нужен Rosetta 2:
```bash
softwareupdate --install-rosetta
```

### Linux (Ubuntu/Debian)

**Требования:**
- Build essentials (gcc, make)
- Ассемблер NASM

**Примечание:** Linux по умолчанию использует `-terminal`, создавая ELF 32-битные бинарники с линковкой на libc.

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

Смотрите инструкции по установке выше для вашей платформы, или:

```bash
cd compiler
make
./defacto -h
```

## Использование

**macOS** (по умолчанию: `-terminal-macos`):
```bash
./defacto hello.de -o hello
./hello
```

**Linux** (по умолчанию: `-terminal`):
```bash
./defacto hello.de -o hello
./hello
```

**Bare-metal ядро** (все платформы):
```bash
./defacto -kernel os.de -o kernel.bin
```

### Командная строка

Компилировать программу:
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
|------|----------|--------|---------|
| `-kernel` | Все | Binary (x86-32) | Linux |
| `-terminal` | Linux | ELF 32-bit | Linux |
| `-terminal-macos` | macOS | Mach-O 64-bit | macOS |

**Выбор режима:**
- **macOS**: По умолчанию `-terminal-macos` (нативные macOS бинарники)
- **Linux**: По умолчанию `-terminal` (Linux syscall)
- **Bare-metal**: Используйте `-kernel` для разработки ОС (без зависимостей ОС)

### Примеры

```bash
# Нативное приложение macOS
./defacto app.de -o app
./app

# Приложение Linux (на Linux)
./defacto app.de -o app
./app

# Bare-metal ядро
./defacto -kernel kernel.de -o kernel.bin

# Запустить в QEMU
qemu-system-i386 -kernel kernel.bin
```

## Полный синтаксис языка

### Обязательные директивы файла

```de
#Mainprogramm.start
#NO_RUNTIME
#SAFE

#Mainprogramm.end
```

Примечания:

- `#Mainprogramm.start` / `#Mainprogramm.end` обязательны.
- `#NO_RUNTIME` обязателен для bare-metal режима.
- `#SAFE` зарезервирована; проверки памяти всегда включены.
- `#INTERRUPT {num} == func` парсится, но ещё не генерируется.

### Секции

Весь код должен быть внутри секций:

```de
<.de
    var x: i32 = 0
    var msg: string = "hello"

    display{msg}
    x = (x + 1)
.>
```

Правила:

- Переменные (`var`) и константы (`const`) можно объявлять в любом месте секции
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
| `pointer` | 4 байта | сырой указатель |
| `struct_name` | варьируется | пользовательский тип структуры |

Массивы:

```de
var buf: u8[64]
var arr: i32[10]
```

Правила:

- `const` массивы не поддерживаются.
- `const` значения должны быть инициализированы.

### Структуры

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

**Массивы в структурах (v0.46+):**

```de
struct Buffer {
    data: u8[256]
    size: i32
}

var buf: Buffer
buf.size = 10
```

Правила:

- Поля структур могут быть любого встроенного типа.
- Структуры хранятся inline (без указателей).
- Доступ к полям через точку: `struct.field`

### Выражения

Поддерживаются полные вложенные выражения:

```de
x = (a + b + c)
x = ((a + b) * c)
x = -5
result = (a * b) + (c / d)
```

Поддерживаемые операторы:

| Оператор | Описание | Пример |
| --- | --- | --- |
| `+` | Сложение | `x = (a + b)` |
| `-` | Вычитание | `x = (x - 1)` |
| `*` | Умножение | `x = (x * 2)` |
| `/` | Деление | `x = (x / 4)` |

### Инструкции

#### Присваивание

```de
x = 1
x = other
x = (x + 1)
arr[i] = x
p.x = 100
```

#### Цикл loop

```de
loop {
    if x == 10 { stop }
}
```

#### If/Else

```de
if x == y {
    display{msg}
} else {
    display{err}
}
```

Все операторы сравнения поддерживаются:

| Оператор | Пример | Описание |
|----------|---------|-------------|
| `==` | `if x == y` | Равно |
| `!=` | `if x != y` | Не равно |
| `<` | `if x < y` | Меньше |
| `>` | `if x > y` | Больше |
| `<=` | `if x <= y` | Меньше или равно |
| `>=` | `if x >= y` | Больше или равно |

Правила:

- Блок `else` опционален.

#### Цикл For

```de
for i = 0 to 10 {
    display{i}
}
```

#### Цикл While

```de
while x < y {
    x = (x + 1)
}
```

#### Цикл Loop

```de
loop {
    if x == 10 { stop }
}
```

#### Switch/Case (v0.46+)

```de
switch x {
    case 1:
        display{"one"}
    case 5:
        display{"five"}
    default:
        display{"other"}
}
```

### Функции

```de
fn my_func {
    <.de
        var a: i32 = 0
        a = (a + 1)
        display{a}
    .>
}

call #my_func
```

**С параметрами:**

```de
fn add(a: i32, b: i32) {
    <.de
        var result: i32 = 0
        result = (a + b)
        display{result}
    .>
}

call #add
```

### Драйверы

```de
driver keyboard {
    type = keyboard
}

// Или проще — тип определяется по имени:
driver mouse

call #keyboard
call #mouse
```

Поддерживаемые типы драйверов: `keyboard`, `mouse`, `volume`

### Extern функции (v0.46+)

Вызов функций из C библиотек:

```de
extern printf
extern malloc
extern free
```

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

- `display{string}` выводит строку. В terminal режиме добавляет новую строку.
- `readkey` и `readchar` работают только в bare-metal режиме. В terminal режиме возвращают `0`.
- `readchar` поддерживает ASCII `a-z`, `0-9`, пробел, enter, backspace.
- `color`, `putchar`, `clear`, `reboot` работают только в bare-metal режиме.

### Регистры

```de
#MOV {#R1, x}
#MOV {x, #R1}
#R1 = (#R1 + #R2)
```

Примечания:

- Отображение регистров фиксировано на x86-32.
- `#STATIC` парсится, но ещё не генерируется.

## Управление памятью

**Автоматическое управление памятью:**

Компилятор автоматически освобождает все переменные в конце каждой секции. Не нужно вызывать `free{}` вручную.

```de
<.de
    var x: i32 = 0
    var msg: string = "hello"
    display{msg}
.>
```

Легаси `free{}` всё ещё поддерживается.

## Комментарии и строки

- Строковые комментарии используют `//`.
- Строки поддерживают экранирование `\n` и `\t`.

```de
var msg: string = "Hello\nWorld"
```

## Пакетный менеджер defo

```
./defo init my-app
./defo install https://github.com/owner/repo.git
./defo install owner/repo
./defo install owner/repo#main
./defo list
```

## Что можно создавать на Defacto?

Defacto создан для низкоуровневого системного программирования.

**Подходит для:**

| Тип проекта | Пример | Сложность |
|-------------|---------|-----------|
| **Операционные системы** | Bare-metal ядра, загрузчики | Высокая |
| **Встраиваемые системы** | Микроконтроллеры, IoT устройства | Средняя |
| **Системные утилиты** | Дисковые инструменты, мониторы железа | Средняя |
| **Образовательные проекты** | Изучение OS dev, компиляторов | Начальная |
| **Демосцена** | 64kb интро, демосцена продюкшены | Высокая |
| **Ретро вычисления** | DOS-подобные программы | Начальная |

**Не подходит для:**

- Веб-приложения (используйте Rust, Go, или JavaScript)
- Мобильные приложения (используйте Swift, Kotlin, или Flutter)
- Data science (используйте Python или Julia)
- Desktop GUI приложения (используйте C#, Java, или Electron)

**Примеры проектов:**

```de
// Загрузчик
#Mainprogramm.start
#NO_RUNTIME
#SAFE

<.de
    var msg: string = "Загрузка..."
    display{msg}
    reboot{}
.>

#Mainprogramm.end
```

```de
// Простое ядро ОС
driver keyboard {
    type = keyboard
}

fn main {
    <.de
        var key: i32 = 0
        loop {
            call #keyboard
            readkey{key}
            if key != 0 {
                putchar{key}
            }
        }
    .>
}
```

---

## Ограничения

- Нет индексов выражений в массивах: `arr[i + 1]` не поддерживается
- Директивы прерываний парсятся, но не генерируются

## Производительность

Defacto компилируется в нативный x86-32 код с минимальными оверхедами рантайма.

**Сравнение (приблизительно):**

| Язык | Относительная скорость | Примечания |
|------|----------------------|------------|
| Ассемблер (ручная оптимизация) | 1.0x | База |
| C (gcc -O3) | 1.0-1.2x | Зрелый оптимизатор |
| Rust | 1.0-1.3x | LLVM бэкенд |
| Zig | 1.1-1.4x | LLVM бэкенд |
| **Defacto v0.45** | 2-5x | Простая генерация кода, без оптимизаций |
| Go | 2-5x | GC оверхед |
| Python | 50-100x | Интерпретатор |

**Почему Defacto быстрый:**
- Компиляция в нативный код (нет VM, нет интерпретатора)
- Нет сборщика мусора
- Прямые системные вызовы в terminal режиме
- Минимальный рантайм (~нулевой оверхед)

**Почему Defacto медленнее C/Rust/Zig:**
- Нет LLVM бэкенда (использует NASM напрямую)
- Нет продвинутых оптимизаций (инлайн, векторизация, и т.д.)
- Компилятор на ранней стадии

Для bare-metal и системного программирования производительность ограничена качеством вашего кода, а не языком.

## Аддоны

Расширьте Defacto нативными библиотеками!

### Rust аддоны

Пишите высокопроизводительные аддоны на Rust с безопасностью памяти:

```bash
cd addons/rust
cargo build --release
```

Смотрите [`addons/rust/README.md`](addons/rust/README.md) для полного руководства.

### Backend фреймворк (Rust)

HTTP веб-фреймворк для создания серверов:

```bash
cd addons/rust-backend
cargo build --release
```

Смотрите [`addons/rust-backend/README.md`](addons/rust-backend/README.md) для API документации.

### C++ аддоны

Традиционные C++ аддоны также поддерживаются:

```bash
cd addons/cpp
make
```

Смотрите [`addons/cpp/`](addons/cpp/) для примеров.
