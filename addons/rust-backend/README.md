# Defacto Backend Framework

Веб-фреймворк для создания HTTP-серверов на Defacto с использованием Rust.

## Возможности

- ✅ HTTP сервер (GET/POST)
- ✅ Маршрутизация (роутинг)
- ✅ JSON ответы
- ✅ Асинхронность (tokio)

## Быстрый старт

### 1. Сборка

```bash
cd addons/rust-backend
make
```

### 2. Использование в Defacto

```de
#Mainprogramm.start
#NO_RUNTIME
#SAFE

<.de
    // Инициализация сервера на порту 8080
    // server_init(8080)
    
    // Добавление GET маршрута
    // route_get("/hello", handler)
    
    // Запуск сервера
    // server_start()
    
    static.pl>
    // Сервер запущен, открывай http://localhost:8080
.>

#Mainprogramm.end
```

## API

### `server_init(port: i32) -> i32`

Инициализирует HTTP сервер на указанном порту.

```de
var result: i32 = 0
result = server_init(8080)
```

### `route_get(path: string, handler: fn) -> i32`

Добавляет GET маршрут.

```de
route_get("/hello", hello_handler)
route_get("/api/users", users_handler)
```

### `route_post(path: string, handler: fn) -> i32`

Добавляет POST маршрут.

```de
route_post("/api/data", data_handler)
```

### `server_start()`

Запускает сервер (блокирующая функция).

```de
server_start()
// Сервер работает пока программа не будет остановлена
```

### `backend_version() -> string`

Возвращает версию фреймворка.

```de
var version: string = backend_version()
display{version}  // "0.1.0"
```

## Примеры

### Простой сервер

```de
#Mainprogramm.start
#NO_RUNTIME
#SAFE

<.de
    var port: i32 = 0
    port = server_init(8080)
    
    route_get("/", home_handler)
    route_get("/hello", hello_handler)
    
    server_start()
    
    static.pl>
.>

#Mainprogramm.end
```

### Обработчик маршрута

```de
function == home_handler {
    <.de
        var response: string = ""
        // Возвращаем JSON
        response = json_object()
        static.pl>
    .>
}
```

## Структура проекта

```
rust-backend/
├── Cargo.toml      # Rust зависимости
├── Makefile        # Сборка
├── README.md       # Документация
└── src/
    └── lib.rs      # Исходный код
```

## Зависимости

- [tokio](https://tokio.rs/) - асинхронная runtime
- [hyper](https://hyper.rs/) - HTTP библиотека
- [serde](https://serde.rs/) - сериализация
- [serde_json](https://github.com/serde-rs/json) - JSON

## Сборка

```bash
# Release (оптимизированная)
cargo build --release

# Debug (для отладки)
cargo build

# Тесты
cargo test

# Очистка
cargo clean
```

## Планы

- [ ] Поддержка параметров маршрута (`/users/:id`)
- [ ] Middleware система
- [ ] Работа с запросами (body, headers)
- [ ] Статические файлы
- [ ] Шаблоны ответов

## Лицензия

MIT
