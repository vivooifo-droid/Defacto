# Defacto v0.44 — Полная документация по синтаксису

## Оглавление

1. [Структура файла](#структура-файла)
2. [Директивы компилятора](#директивы-компилятора)
3. [Секции кода](#секции-кода)
4. [Типы данных](#типы-данных)
5. [Переменные и константы](#переменные-и-константы)
6. [Структуры](#структуры)
7. [Выражения](#выражения)
8. [Инструкции](#инструкции)
9. [Функции](#функции)
10. [Драйверы](#драйверы)
11. [Встроенные функции](#встроенные-функции)
12. [Регистры](#регистры)
13. [Комментарии и строки](#комментарии-и-строки)
14. [Управление памятью](#управление-памятью)
15. [Ограничения языка](#ограничения-языка)

---

## Структура файла

Каждый файл Defacto должен иметь следующую структуру:

```de
#Mainprogramm.start
#NO_RUNTIME
#SAFE


struct MyStruct {
    field: i32
}

// Новый синтаксис драйверов (v0.44+)
driver keyboard {
    type = keyboard
}

fn my_func {
    <.de
        var x: i32 = 0
        x = (x + 1)
    .>
}


<.de
    var x: i32 = 0
    display{x}
.>

#Mainprogramm.end
```

### Порядок элементов

1. Директивы (`#NO_RUNTIME`, `#SAFE`)
2. Структуры (`struct`)
3. Драйверы (`<drv.` ... `.dr>`)
4. Функции (`fn` или `function ==`)
5. Основная секция (`<.de` ... `.>`)
6. Завершающая директива (`#Mainprogramm.end`)

---

## Директивы компилятора

### `#Mainprogramm.start`

**Обязательная.** Начинает программу. Должна быть первой строкой.

### `#Mainprogramm.end`

**Обязательная.** Завершает программу. Должна быть последней строкой.

### `#NO_RUNTIME`

**Обязательная для bare-metal.** Отключает стандартную среду выполнения.
Используется для ОС и загрузчиков.

### `#SAFE`

Зарезервирована. Проверки памяти всегда включены.

### `#DRIVER` / `#DRIVER.stop`

Альтернатива `#NO_RUNTIME` для программ с драйверами.

---

## Секции кода

### Основная секция `<.de` ... `.>`

Вся исполняемая программа должна быть внутри этой секции.

**Новое в v0.44:** Объявления и инструкции можно смешивать свободно:

```de
<.de
    var x: i32 = 0
    var msg: string = "hello"
    
    display{msg}
    x = (x + 1)
    
    var y: i32 = 10  // Можно объявлять переменные в любом месте
    y = (y * 2)
.>
```

**Обратная совместимость:** `static.pl>` всё ещё работает:

```de
<.de
    var x: i32 = 0
    var msg: string = "hello"

    static.pl>

    display{msg}
    x = (x + 1)
.>
```

### Правило двух частей (устарело в v0.44+)

Раньше секция делилась на две части через `static.pl>`:

| До `static.pl>` | После `static.pl>` |
| --- | --- |
| Только `var` и `const` | Только инструкции |
| Объявления переменных | Вызовы функций, присваивания |
| Нет исполняемого кода | Нет объявлений |

**В v0.44+:** Это ограничение снято.

---

## Типы данных

### Встроенные типы

| Тип | Размер | Диапазон | Описание |
| --- | --- | --- | --- |
| `i32` | 4 байта | -2³¹ ... 2³¹-1 | Знаковое целое |
| `i64` | 8 байт | -2⁶³ ... 2⁶³-1 | Знаковое целое |
| `u8` | 1 байт | 0 ... 255 | Беззнаковый байт |
| `string` | 4 байта | - | Указатель на строку |
| `pointer` | 4 байта | - | Сырой указатель |

### Пользовательские типы (структуры)

```de
struct Point {
    x: i32
    y: i32
}

var p: Point
```

---

## Переменные и константы

### Объявление переменных

```de

var count: i32 = 42
var msg: string = "hello"


var x: i32
var buf: u8[64]
```

### Константы

```de
const MAX: i32 = 100
const NAME: string = "Defacto"
```

**Правила:**
- `const` **должна** иметь инициализатор
- `const` **нельзя** изменять
- `const` **нельзя** освобождать через `free{}`

### Массивы

```de
var arr: i32[10]
var buf: u8[256]
```

**Важно:**
- `const` массивы не поддерживаются
- Индексация: `arr[i] = x`
- Индекс должен быть: число, переменная или регистр

---

## Структуры

### Определение структуры

```de
struct Player {
    x: i32
    y: i32
    health: i32
    score: i32
}
```

### Объявление переменной-структуры

```de
var player: Player
```

### Доступ к полям

```de
player.x = 10
player.y = 20
player.health = 100
player.score = 500
```

### Расположение в памяти

Поля располагаются последовательно:

```
Player (16 байт):
+0: x (i32)
+4: y (i32)
+8: health (i32)
+12: score (i32)
```

### Размеры полей

| Тип поля | Размер |
| --- | --- |
| `i32` | 4 байта |
| `i64` | 8 байт |
| `u8` | 1 байт |
| `string` | 4 байта (указатель) |
| `pointer` | 4 байта |

### Вложенные структуры

**Не поддерживаются в v0.30.** Поля могут быть только встроенных типов.

---

## Выражения

### Новое в v0.44

**Полная поддержка сложных выражений:**

```de
x = (a + b + c)
x = ((a + b) * c)
x = -5
result = (a * b) + (c / d)
```

### Поддерживаемые операторы

| Оператор | Описание | Пример |
| --- | --- | --- |
| `+` | Сложение | `x = (a + b)` |
| `-` | Вычитание | `x = (x - 1)` |
| `*` | Умножение | `x = (x * 2)` |
| `/` | Деление | `x = (x / 4)` |

### Правила выражений

**v0.44+:**
- ✅ Вложенные выражения поддерживаются
- ✅ Отрицательные литералы поддерживаются
- ✅ Несколько операторов в одном выражении

```de
x = (a + b)
x = (x * 2)
x = -5  // Отрицательный литерал
result = (a + b + c)  // Несколько операторов
```

### Присваивание выражений

```de
result = (a + b)
```

---

## Инструкции

### Присваивание

```de

x = 5
x = other_var
x = #R1


x = (x + 1)
y = (a * b)


arr[i] = x
arr[0] = 100


player.x = 10
player.health = (player.health - 1)
```

### Цикл `loop`

```de
loop {

    counter = (counter - 1)
    

    if counter == 0 { stop }
}
```

**Правила:**
- `loop` создаёт бесконечный цикл
- `stop` выходит из цикла
- Используйте `if` для условия выхода

### Условие `if/else`

```de
if x == 10 {
    display{msg}
}

if x == 10 {
    display{msg}
} else {
    display{err}
}

if x == 1 {
    if y == 2 {
        stop
    }
}
```

**Операторы сравнения (v0.44+):**

| Оператор | Пример | Описание |
| --- | --- | --- |
| `==` | `if x == y` | Равно |
| `!=` | `if x != y` | Не равно |
| `<` | `if x < y` | Меньше |
| `>` | `if x > y` | Больше |
| `<=` | `if x <= y` | Меньше или равно |
| `>=` | `if x >= y` | Больше или равно |

**Правила:**
- `else` блок опционален
- Все операторы сравнения поддерживаются

### Цикл `for` (v0.44+)

**Новый синтаксис:**

```de
for i = 0 to 10 {
    display{i}
}
```

**Старый синтаксис (всё ещё работает):**

```de
for i = 0; i < 10; i = (i + 1) {
    display{i}
}
```

### Цикл `while`

```de
while x < y {
    x = (x + 1)
}
```

### Цикл `loop`

```de
loop {
    counter = (counter - 1)
    
    if counter == 0 { stop }
}
```

**Правила:**
- `loop` создаёт бесконечный цикл
- `stop` выходит из цикла

### `stop`

Выход из цикла или программы:

```de
loop {
    if x == 10 { stop }
}
```

---

## Функции

### Объявление функции (v0.44+)

**Новый упрощённый синтаксис:**

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

**Старый синтаксис (всё ещё работает):**

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

### Вызов функции

```de
call #my_func
```

### Правила

- Функции объявляются **до** основной секции
- Вызов через `call #имя`
- Каждая функция имеет свою секцию `<.de` ... `.>`
- Локальные переменные внутри функции

---

## Драйверы

### Новый синтаксис (v0.44+)

**Простое объявление:**
```de
driver keyboard

call #keyboard
```

**С явным указанием типа:**
```de
driver keyboard {
    type = keyboard
}

call #keyboard
```

**Своё имя с типом:**
```de
driver my_keyboard {
    type = keyboard
}

call #my_keyboard
```

### Старый синтаксис (всё ещё работает)

```de
<drv.
Const.driver = keyboard_driver
keyboard_driver <<func = keyboard>>
.dr>

call #keyboard_driver
```

### Компоненты старого синтаксиса

| Часть | Описание |
| --- | --- |
| `<drv.` ... `.dr>` | Блок драйвера |
| `Const.driver = имя` | Объявление константы драйвера |
| `имя <<func = тип>>` | Привязка функции драйвера |

### Типы драйверов

| Тип | Описание |
| --- | --- |
| `keyboard` | Клавиатура (PS/2) |
| `mouse` | Мышь (PS/2) |
| `volume` | PC Speaker |

### Вызов драйвера

```de
call #keyboard       // Новый синтаксис
call #keyboard_driver // Старый синтаксис
```

### Встроенные драйверы

Компилятор генерирует код для встроенных драйверов автоматически.

---

## Встроенные функции

### Ввод/Вывод

| Функция | Описание | Режим |
| --- | --- | --- |
| `display{var}` | Вывод строки | Все |
| `readkey{var}` | Чтение скан-кода | Bare-metal |
| `readchar{var}` | Чтение ASCII | Bare-metal |
| `putchar{code}` | Вывод символа | Bare-metal |

### Графика (Bare-metal)

| Функция | Описание |
| --- | --- |
| `color{attr}` | Установить атрибут цвета |
| `clear{}` | Очистить экран |

### Системные

| Функция | Описание | Режим |
| --- | --- | --- |
| `reboot{}` | Перезагрузка | Bare-metal |

### Примеры

```de

var msg: string = "Hello"
static.pl>
display{msg}


var ch: i32 = 0
static.pl>
readchar{ch}
display{ch}


color{10}
display{msg}


clear{}


reboot{}
```

---

## Регистры

### Доступные регистры

| Регистр | x86 | Описание |
| --- | --- | --- |
| `#R1` | EDI | Регистр 1 |
| `#R2` | ESI | Регистр 2 |
| `#R3` | EDX | Регистр 3 |
| `#R4` | ECX | Регистр 4 |
| `#R5` | EBX | Регистр 5 |
| `#R6` | EAX | Регистр 6 (аккумулятор) |
| `#R7` | EDI | Регистр 7 |
| `#R8` | ESI | Регистр 8 |
| `#R9` | EBX | Регистр 9 |
| `#R10` | ECX | Регистр 10 |
| `#R11` | EDX | Регистр 11 |
| `#R12` | ESI | Регистр 12 |
| `#R13` | EDI | Регистр 13 |
| `#R14` | EAX | Регистр 14 |
| `#R15` | EBP | Регистр 15 (базовый указатель) |
| `#R16` | ESP | Регистр 16 (указатель стека) |

### Операции с регистрами

```de

#MOV {#R1, x}


#MOV {x, #R1}


#R1 = (#R1 + #R2)
#R3 = (#R3 - 5)
```

### Правила

- Регистры начинаются с `#`
- Фиксированное отображение на x86-32
- `#STATIC` парсится, но не генерируется

---

## Комментарии и строки

### Комментарии

```de

var x: i32 = 0
```

### Строковые литералы

```de
var msg: string = "Hello, World!"
var multiline: string = "Line 1\nLine 2"
var with_tab: string = "Col1\tCol2"
```

### Escape-последовательности

| Последовательность | Символ |
| --- | --- |
| `\n` | Новая строка (LF) |
| `\t` | Табуляция |

---

## Управление памятью

### Автоматическое управление (v0.30+)

Компилятор **автоматически** освобождает все переменные в конце секции:

```de
<.de
    var x: i32 = 0
    var msg: string = "hello"
    static.pl>
    display{msg}
.>

```

### Ручное освобождение (legacy)

```de
<.de
    var x: i32 = 0
    static.pl>
    free{x}
.>
```

### Правила

| Тип | Освобождение |
| --- | --- |
| `var` | Автоматически или `free{}` |
| `const` | Нельзя освобождать |
| `string` | Автоматически (статические данные) |
| `array` | Автоматически |

---

## Ограничения языка

### Исправлено в v0.44

✅ **Все операторы сравнения работают:** `==`, `!=`, `<`, `>`, `<=`, `>=`

✅ **Вложенные выражения поддерживаются:** `(a + b + c)`, `((a + b) * c)`

✅ **Отрицательные литералы поддерживаются:** `-5`, `-42`

✅ **`static.pl>` теперь опционален**

### Выражения

- ✅ Вложенные выражения: `(a + b + c)`
- ✅ Отрицательные литералы: `-5`
- ✅ Несколько операторов в выражении

### Условия

- ✅ Все операторы сравнения: `==`, `!=`, `<`, `>`, `<=`, `>=`
- ❌ Нет `elseif`
- ✅ `else` поддерживается

### Массивы

- ❌ Нет `const` массивов
- ❌ Нет выражений в индексах: `arr[i + 1]`
- ✅ Индекс: число, переменная, регистр

### Структуры

- ❌ Нет вложенных структур
- ❌ Нет методов у структур
- ✅ Поля: только встроенные типы

### Прочее

- ❌ Interrupt директивы не генерируются
- ❌ `#STATIC` не генерируется

---

## Примеры программ

### Hello World

```de
#Mainprogramm.start
#NO_RUNTIME
#SAFE

<.de
    var msg: string = "Hello, World!"
    display{msg}
.>

#Mainprogramm.end
```

### Калькулятор

```de
#Mainprogramm.start
#NO_RUNTIME
#SAFE

<.de
    var a: i32 = 10
    var b: i32 = 5
    var result: i32 = 0
    static.pl>

    result = (a + b)
    result = (a - b)
    result = (a * b)
    result = (a / b)

.>

#Mainprogramm.end
```

### Структуры

```de
#Mainprogramm.start
#NO_RUNTIME
#SAFE

struct Player {
    x: i32
    y: i32
    health: i32
    score: i32
}

<.de
    var player: Player
    static.pl>

    player.x = 100
    player.y = 200
    player.health = 100
    player.score = 0

    player.health = (player.health - 10)
    player.score = (player.score + 100)
.>

#Mainprogramm.end
```

### if/else со всеми операторами сравнения

```de
#Mainprogramm.start
#NO_RUNTIME
#SAFE

<.de
    var x: i32 = 5
    var msg: string = "equal"
    var err: string = "not equal"
    static.pl>

    if x == 5 {
        display{msg}
    } else {
        display{err}
    }
    
    if x != 10 {
        display{"x is not 10"}
    }
    
    if x > 3 {
        display{"x is greater than 3"}
    }
.>

#Mainprogramm.end
```

### for цикл

```de
#Mainprogramm.start
#NO_RUNTIME
#SAFE

<.de
    var i: i32 = 0
    static.pl>

    for i = 0 to 10 {
        display{i}
    }
.>

#Mainprogramm.end
```

### Цикл

```de
#Mainprogramm.start
#NO_RUNTIME
#SAFE

<.de
    var counter: i32 = 10
    static.pl>
    
    loop {
        if counter == 0 { stop }
        counter = (counter - 1)
    }
.>

#Mainprogramm.end
```

### Драйвер клавиатуры

**Новый синтаксис (v0.44+):**
```de
#Mainprogramm.start
#NO_RUNTIME
#SAFE

driver keyboard {
    type = keyboard
}

<.de
    var key: i32 = 0
    var msg: string = "Press a key"

    display{msg}
    call #keyboard
    readkey{key}
    display{key}
.>

#Mainprogramm.end
```

**Старый синтаксис (всё ещё работает):**
```de
#Mainprogramm.start
#NO_RUNTIME
#SAFE

<drv.
Const.driver = keyboard_driver
keyboard_driver <<func = keyboard>>
.dr>

<.de
    var key: i32 = 0
    static.pl>

    call #keyboard_driver
    readkey{key}
    display{key}
.>

#Mainprogramm.end
```

---

## Компиляция и запуск

### Компиляция

```bash
# В бинарный файл
./defacto program.de

# Только ассемблер
./defacto -S program.de

# С указанием выхода
./defacto -o output.bin program.de

# Подробный вывод
./defacto -v program.de
```

### Режимы

```bash
# Bare-metal (по умолчанию)
./defacto -kernel program.de

# Linux terminal
./defacto -terminal program.de

# macOS terminal
./defacto -terminal-macos program.de
```

---

## Отладка

### Сообщения об ошибках

```
error: undefined variable 'x'
error: expected type at line 10
error: cannot assign to const 'MAX'
```

### Предупреждения

```
warning: unexpected token '+', skipping
```

### Проверка памяти

```
error: Memory Abandonment: 'x' never freed
```

(В v0.30+ автоматически освобождается)
