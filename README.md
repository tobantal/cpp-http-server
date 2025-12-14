# 🌐 cpp-http-server

Современная библиотека HTTP-сервера на C++17 с чистой архитектурой и двумя модульными компонентами.

#### Автор: Тоболкин Антон

---

## 📦 Архитектура

Библиотека состоит из **двух независимых модулей**:

### 🎯 Core Module (`http-server-core`)
**Чистые абстракции и интерфейсы** — нулевые зависимости, header-only.

| Компонент | Назначение |
|-----------|-----------|
| `IRequest` | Интерфейс HTTP-запроса (путь, метод, тело, заголовки, параметры) |
| `IResponse` | Интерфейс HTTP-ответа (статус, тело, заголовки) |
| `IWebApplication` | Базовый класс приложения с паттерном Template Method |
| `IHttpHandler` | Интерфейс обработчика маршрутов |
| `IHttpClient` | HTTP-клиент для межсервисной коммуникации |
| `IEnvironment` | Интерфейс управления конфигурацией |
| `RouteMatcher` | Сопоставление маршрутов с подстановочными символами |
| `Environment` | Объект конфигурации с type-safe геттерами |
| `SimpleRequest/Response` | Минималистичные реализации для тестирования |

**Зачем отдельно?** Используйте core-интерфейсы в своих сервисах без линковки Boost.

### ⚡ Boost Module (`http-server-boost`)
**Production-ready HTTP-сервер** на основе Boost.Beast и Boost.Asio.

| Компонент | Назначение |
|-----------|-----------|
| `BoostBeastApplication` | Полнофункциональный HTTP-сервер с асинхронным I/O |
| `BeastRequestAdapter` | Адаптер Beast-запросов к `IRequest` |
| `BeastResponseAdapter` | Адаптер Beast-ответов к `IResponse` |
| `HttpClient` | HTTP-клиент на Beast для сервис-сервис коммуникации |
| `ServerSettings` | Конфигурация хоста/порта сервера из Environment |
| `DbSettings` | Параметры подключения БД из Environment |

---

## 🚀 Установка

### Через CMake FetchContent

```cmake
cmake_minimum_required(VERSION 3.15)
project(my_service)

include(FetchContent)

FetchContent_Declare(http_server
  GIT_REPOSITORY https://github.com/YOUR_USERNAME/cpp-http-server.git
  GIT_TAG v1.0.0
)

FetchContent_MakeAvailable(http_server)

add_executable(my_app src/main.cpp)
target_link_libraries(my_app http_server)
```

### Требования

- **C++17** или выше
- **CMake 3.15+**
- **Boost 1.70+** (для модуля http-server-boost)
  - `Boost.Asio`
  - `Boost.Beast`
- **nlohmann/json** (для парсинга JSON конфигурации)

### Сборка из исходников

```bash
git clone https://github.com/YOUR_USERNAME/cpp-http-server.git
cd cpp-http-server
mkdir build && cd build
cmake ..
cmake --build .
ctest  # Запустить unit-тесты
```

---

## 💡 Быстрый старт

### 1️⃣ Создайте класс приложения

```cpp
#include "BoostBeastApplication.hpp"
#include "IHttpHandler.hpp"
#include "IRequest.hpp"
#include "IResponse.hpp"

class MyWebApp : public BoostBeastApplication {
public:
    void configureInjection() override {
        // Регистрируем обработчики маршрутов
        registerHandler("GET", "/status", 
            std::make_shared<StatusHandler>());
        
        registerHandler("POST", "/api/users", 
            std::make_shared<CreateUserHandler>());
        
        registerHandler("GET", "/api/users/*", 
            std::make_shared<GetUserHandler>());
    }

protected:
    void registerHandler(const std::string& method, 
                        const std::string& path,
                        std::shared_ptr<IHttpHandler> handler) {
        handlers_[getHandlerKey(method, path)] = handler;
    }
};
```

### 2️⃣ Реализуйте обработчики

```cpp
#include "IHttpHandler.hpp"

class StatusHandler : public IHttpHandler {
public:
    void handle(IRequest& req, IResponse& res) override {
        res.setStatus(200);
        res.setHeader("Content-Type", "application/json");
        res.setBody(R"({"status":"running"})");
    }
};

class CreateUserHandler : public IHttpHandler {
public:
    void handle(IRequest& req, IResponse& res) override {
        try {
            std::string body = req.getBody();
            // Парсим JSON и создаём пользователя...
            
            res.setStatus(201);
            res.setHeader("Content-Type", "application/json");
            res.setBody(R"({"id":123,"name":"John"})");
        } catch (const std::exception& e) {
            res.setStatus(400);
            res.setBody(R"({"error":"Bad request"})");
        }
    }
};
```

### 3️⃣ Файл конфигурации

**config.json**
```json
{
  "server": {
    "host": "0.0.0.0",
    "port": 8080
  },
  "db": {
    "host": "localhost",
    "port": 5432,
    "name": "myapp",
    "user": "postgres",
    "password": "secret"
  }
}
```

### 4️⃣ Функция main

```cpp
#include <iostream>

int main(int argc, char* argv[]) {
    try {
        MyWebApp app;
        app.run(argc, argv);  // Блокирует до остановки сервера
    } catch (const std::exception& e) {
        std::cerr << "Ошибка: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}
```

### 5️⃣ Сборка и запуск

```bash
mkdir build && cd build
cmake ..
cmake --build .
./my_app
# Сервер слушает на 0.0.0.0:8080
```

---

## 📡 Справка по API

### Интерфейс запроса

```cpp
struct IRequest {
    virtual std::string getPath() const = 0;        // "/api/users?id=10"
    virtual std::string getMethod() const = 0;      // "GET", "POST", и т.д.
    virtual std::string getBody() const = 0;        // Тело запроса
    virtual std::map<std::string, std::string> 
        getParams() const = 0;                      // Query параметры: {"id": "10"}
    virtual std::map<std::string, std::string> 
        getHeaders() const = 0;                     // HTTP заголовки
    virtual std::string getIp() const = 0;          // IP клиента
    virtual int getPort() const = 0;                // Порт клиента
};
```

### Интерфейс ответа

```cpp
struct IResponse {
    virtual void setStatus(int code) = 0;           // 200, 404, 500, и т.д.
    virtual void setBody(const std::string& body) = 0;
    virtual void setHeader(const std::string& name, 
                          const std::string& value) = 0;
};
```

### Интерфейс обработчика

```cpp
class IHttpHandler {
public:
    virtual void handle(IRequest& req, IResponse& res) = 0;
};
```

---

## 🛣️ Сопоставление маршрутов

Маршруты поддерживают **подстановочные паттерны** для динамических сегментов:

```cpp
registerHandler("GET", "/api/users/123", handler);      // Точное совпадение
registerHandler("GET", "/api/users/*", handler);        // Любой пользователь
registerHandler("GET", "/*/users/*/profile", handler);  // Несколько сегментов
```

### Примеры сопоставления паттернов

| Паттерн | Совпадает | Не совпадает |
|---------|-----------|-------------|
| `/api/users` | `/api/users` | `/api/users/123` |
| `/api/users/*` | `/api/users/123` | `/api/users/123/edit` |
| `/api/*/details` | `/api/users/details` | `/api/users/123/details` |
| `/*/users/*` | `/v1/users/123` | `/users/123` |

---

## ⚙️ Конфигурация

### Конфигурация на основе Environment

Автоматически загружает `config.json` и внедряет значения:

```cpp
auto settings = std::make_shared<ServerSettings>(env_);
std::string host = settings->getHost();      // "0.0.0.0"
int port = settings->getPort();              // 8080

auto dbSettings = std::make_shared<DbSettings>(env_);
std::string dbHost = dbSettings->getHost();  // "localhost"
int dbPort = dbSettings->getPort();          // 5432
```

### Ручной доступ к Environment

```cpp
std::string apiKey = env_->get<std::string>("api.key");
int timeout = env_->get<int>("server.timeout", 30);  // С значением по умолчанию
```

---

## 📨 HTTP-клиент (Межсервисная коммуникация)

### Отправка запросов между сервисами

```cpp
#include "HttpClient.hpp"
#include "SimpleRequest.hpp"
#include "SimpleResponse.hpp"

HttpClient client;

// Создаём исходящий запрос
SimpleRequest request(
    "GET",                          // метод
    "/api/internal/status",         // путь
    "",                             // тело
    "other-service.local",          // IP/хост получателя
    8080,                           // порт
    {{"Authorization", "Bearer TOKEN"}}  // заголовки
);

SimpleResponse response;

if (client.send(request, response)) {
    std::cout << "Статус: " << response.getStatus() << std::endl;
    std::cout << "Тело: " << response.getBody() << std::endl;
} else {
    std::cerr << "Запрос не удался" << std::endl;
}
```

---

## 🧪 Тестирование

### Запуск unit-тестов

```bash
cd build
cmake ..
cmake --build .
ctest --verbose
```

### Покрытие тестами

- ✅ **BeastRequestAdapter** — парсинг HTTP-запросов и параметров
- ✅ **BeastResponseAdapter** — построение ответов
- ✅ **BoostBeastApplication** — жизненный цикл сервера, маршрутизация
- ✅ **HttpClient** — синхронный HTTP-клиент
- ✅ **RouteMatcher** — сопоставление маршрутов с подстановками
- ✅ **ServerSettings** — загрузка конфигурации с валидацией
- ✅ **DbSettings** — настройки БД из Environment
- ✅ **Environment** — type-safe хранилище свойств

### Напишите свои тесты

```cpp
#include <gtest/gtest.h>
#include "SimpleRequest.hpp"
#include "SimpleResponse.hpp"

TEST(MyHandler, ReturnsJsonResponse) {
    SimpleRequest req("GET", "/api/users", "", "127.0.0.1", 80);
    SimpleResponse res;
    
    MyUserHandler handler;
    handler.handle(req, res);
    
    EXPECT_EQ(res.getStatus(), 200);
    EXPECT_EQ(res.getHeaders()["Content-Type"], "application/json");
}
```

---

## 🏗️ Паттерны проектирования

### 🔌 Паттерн Adapter
`BeastRequestAdapter` и `BeastResponseAdapter` преобразуют объекты Beast в чистые интерфейсы.

```cpp
// Внутренние типы Beast
http::request<http::string_body> beastReq = ...;

// Оборачиваем в адаптер
BeastRequestAdapter adapter(beastReq, clientIp);

// Используем через интерфейс
handler->handle(adapter, response);
```

### 📋 Паттерн Template Method
`IWebApplication::run()` определяет последовательность запуска:

```cpp
void run(int argc, char* argv[]) {
    loadEnvironment(argc, argv);    // ← Переопределите в подклассе
    configureInjection();           // ← Переопределите в подклассе
    start();                        // ← Переопределите в подклассе
}
```

### 🔗 Dependency Injection
Обработчики и настройки регистрируются в `configureInjection()`:

```cpp
void MyWebApp::configureInjection() override {
    auto dbSettings = std::make_shared<DbSettings>(env_);
    auto userRepo = std::make_shared<UserRepository>(dbSettings);
    
    registerHandler("GET", "/api/users", 
        std::make_shared<GetUsersHandler>(userRepo));
}
```

---

## 🐛 Решение проблем

### ❌ "Cannot find Boost libraries"

Установите Boost:
```bash
# macOS
brew install boost

# Ubuntu/Debian
sudo apt-get install libboost-all-dev

# Windows
vcpkg install boost:x64-windows
```

### ❌ "IRequest not found" в коде

Убедитесь, что вы линкуете библиотеку:
```cmake
target_link_libraries(my_app http_server)
```

### ❌ Файл config.json не найден при запуске

Поместите `config.json` в **рабочую директорию** где вы запускаете приложение:
```bash
cd build
cp ../config.json .
./my_app
```

### ❌ Порт уже занят

Измените порт в `config.json`:
```json
{
  "server": {
    "host": "127.0.0.1",
    "port": 9090
  }
}
```

### ❌ Обработчик маршрута не вызывается

Проверьте паттерн маршрута:
```cpp
// ✅ Это совпадает с GET /api/users/123
registerHandler("GET", "/api/users/*", handler);

// ❌ Это совпадает с GET /api/users, но НЕ /api/users/123
registerHandler("GET", "/api/users", handler);
```

---

## 📚 Структура проекта

```
cpp-http-server/
├── include/
│   ├── core/                    # Определения интерфейсов
│   │   ├── IRequest.hpp
│   │   ├── IResponse.hpp
│   │   ├── IHttpHandler.hpp
│   │   ├── IWebApplication.hpp
│   │   └── ...
│   ├── boost/                   # Реализация на Boost.Beast
│   │   ├── BoostBeastApplication.hpp
│   │   ├── BeastRequestAdapter.hpp
│   │   ├── BeastResponseAdapter.hpp
│   │   └── ...
│   └── config/                  # Классы конфигурации
│       ├── Environment.hpp
│       ├── ServerSettings.hpp
│       └── DbSettings.hpp
├── src/
│   ├── BoostBeastApplication.cpp
│   ├── HttpClient.cpp
│   └── RouteMatcher.cpp
├── tests/                       # Unit-тесты
│   ├── BeastRequestAdapterTest.cpp
│   ├── BeastResponseAdapterTest.cpp
│   └── ...
├── CMakeLists.txt
├── config.json
├── README.md
└── LICENSE
```

---

## 📄 Лицензия

MIT License.

---

## 👨‍💻 Вклад

Контрибьюции приветствуются! Пожалуйста:

1. Сделайте fork репозитория
2. Создайте ветку функции (`git checkout -b feature/amazing-feature`)
3. Коммитьте изменения (`git commit -m 'Add amazing feature'`)
4. Отправьте в ветку (`git push origin feature/amazing-feature`)
5. Откройте Pull Request

---

## 📞 Поддержка

- 🐛 [Сообщить об ошибке](https://github.com/tobantal/cpp-http-server/issues)
- 💬 [Обсуждения](https://github.com/tobantal/cpp-http-server/discussions)

---

**Сделано с ❤️ к микросервисной архитектуре**
