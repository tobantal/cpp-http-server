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
| `IRequest` | Интерфейс HTTP-запроса с поддержкой path parameters, query params, headers, attributes |
| `IResponse` | Интерфейс HTTP-ответа с геттерами и convenience методами |
| `IWebApplication` | Базовый класс приложения с паттерном Template Method |
| `IHttpHandler` | Интерфейс обработчика маршрутов |
| `IHttpClient` | HTTP-клиент для межсервисной коммуникации |
| `IEnvironment` | Интерфейс управления конфигурацией |
| `RouteMatcher` | Сопоставление маршрутов с подстановочными символами |
| `Environment` | Объект конфигурации с type-safe геттерами |
| `SimpleRequest/Response` | Полнофункциональные реализации для тестирования |

**Зачем отдельно?** Используйте core-интерфейсы в своих сервисах без линковки Boost.

### ⚡ Boost Module (`http-server-boost`)
**Production-ready HTTP-сервер** на основе Boost.Beast и Boost.Asio.

| Компонент | Назначение |
|-----------|-----------|
| `BoostBeastApplication` | Полнофункциональный HTTP-сервер с поддержкой path parameters |
| `BeastRequestAdapter` | Адаптер Beast-запросов к `IRequest` с case-insensitive headers |
| `BeastResponseAdapter` | Адаптер Beast-ответов к `IResponse` с геттерами |
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

FetchContent_Declare(
    cpp-http-server
    GIT_REPOSITORY https://github.com/tobantal/cpp-http-server.git
    GIT_TAG v0.1.0
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
git clone https://github.com/tobantal/cpp-http-server.git
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
        
        // Path parameters с wildcards
        registerHandler("GET", "/api/users/*",
            std::make_shared<GetUserHandler>());
        
        registerHandler("GET", "/api/orders/*/items/*",
            std::make_shared<GetOrderItemHandler>());
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
        // Используем convenience метод setResult()
        res.setResult(200, "application/json", R"({"status":"running"})");
    }
};

class GetUserHandler : public IHttpHandler {
public:
    void handle(IRequest& req, IResponse& res) override {
        // Извлекаем path parameter (userId из /api/users/*)
        auto userId = req.getPathParam(0);
        
        if (!userId) {
            res.setResult(400, "application/json", R"({"error":"Missing user ID"})");
            return;
        }
        
        // Проверяем Bearer токен
        auto token = req.getBearerToken();
        if (!token) {
            res.setResult(401, "application/json", R"({"error":"Unauthorized"})");
            return;
        }
        
        // Получаем query параметры
        auto fields = req.getQueryParam("fields");  // ?fields=name,email
        
        // Формируем ответ
        res.setResult(200, "application/json", 
            R"({"id":")" + *userId + R"(","name":"John"})");
    }
};

class GetOrderItemHandler : public IHttpHandler {
public:
    void handle(IRequest& req, IResponse& res) override {
        // Несколько path parameters: /api/orders/*/items/*
        auto orderId = req.getPathParam(0);  // ord-123
        auto itemId = req.getPathParam(1);   // item-456
        
        if (!orderId || !itemId) {
            res.setResult(400, "application/json", R"({"error":"Missing parameters"})");
            return;
        }
        
        res.setResult(200, "application/json",
            R"({"orderId":")" + *orderId + R"(","itemId":")" + *itemId + R"("})");
    }
};
```

### 3️⃣ Middleware с атрибутами

```cpp
class AuthMiddleware : public IHttpHandler {
public:
    AuthMiddleware(std::shared_ptr<IHttpHandler> next) : next_(next) {}
    
    void handle(IRequest& req, IResponse& res) override {
        auto token = req.getBearerToken();
        if (!token) {
            res.setResult(401, "application/json", R"({"error":"Unauthorized"})");
            return;
        }
        
        // Валидируем токен и сохраняем данные в атрибутах
        auto userId = validateToken(*token);
        req.setAttribute("user_id", userId);
        req.setAttribute("account_id", "acc-456");
        
        // Передаём управление следующему handler
        next_->handle(req, res);
    }

private:
    std::shared_ptr<IHttpHandler> next_;
};

class OrderHandler : public IHttpHandler {
public:
    void handle(IRequest& req, IResponse& res) override {
        // Получаем данные из middleware
        auto userId = req.getAttribute("user_id");
        auto accountId = req.getAttribute("account_id");
        
        if (!userId || !accountId) {
            res.setResult(500, "application/json", R"({"error":"Auth data missing"})");
            return;
        }
        
        // Используем userId и accountId...
        res.setResult(200, "application/json", R"({"user":")" + *userId + R"("})");
    }
};
```

### 4️⃣ Файл конфигурации

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

### 5️⃣ Функция main

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

---

## 📡 Справка по API

### Интерфейс запроса (IRequest)

```cpp
struct IRequest {
    // === PATH ===
    virtual std::string getPath() const = 0;                    // "/api/users" (без query string)
    virtual std::vector<std::string> getPathSegments() const = 0;  // ["api", "users"]
    
    // === PATH PARAMETERS ===
    virtual std::string getPathPattern() const = 0;             // "/api/users/*"
    virtual void setPathPattern(const std::string& pattern) = 0;
    virtual std::optional<std::string> getPathParam(size_t index) const = 0;  // Wildcard по индексу
    
    // === QUERY PARAMETERS ===
    virtual std::map<std::string, std::string> getQueryParams() const = 0;
    virtual std::optional<std::string> getQueryParam(const std::string& name) const = 0;
    virtual void setQueryParam(const std::string& name, const std::string& value) = 0;
    
    // === HEADERS (case-insensitive) ===
    virtual std::map<std::string, std::string> getHeaders() const = 0;
    virtual std::optional<std::string> getHeader(const std::string& name) const = 0;
    virtual void setHeader(const std::string& name, const std::string& value) = 0;
    virtual void setHeaders(const std::map<std::string, std::string>& headers) = 0;
    
    // === BODY ===
    virtual std::string getBody() const = 0;
    virtual void setBody(const std::string& body) = 0;
    
    // === METHOD & CONNECTION ===
    virtual std::string getMethod() const = 0;                  // "GET", "POST", etc.
    virtual std::string getIp() const = 0;
    virtual int getPort() const = 0;
    
    // === CONVENIENCE METHODS ===
    virtual std::optional<std::string> getBearerToken() const = 0;  // Из Authorization header
    virtual bool isJson() const = 0;                            // Content-Type содержит "json"
    virtual std::string getContentType() const = 0;
    
    // === ATTRIBUTES (для middleware) ===
    virtual void setAttribute(const std::string& name, const std::string& value) = 0;
    virtual std::optional<std::string> getAttribute(const std::string& name) const = 0;
    
    // === DEPRECATED ===
    virtual std::map<std::string, std::string> getParams() const;  // Используйте getQueryParams()
};
```

### Интерфейс ответа (IResponse)

```cpp
struct IResponse {
    // === SETTERS ===
    virtual void setStatus(int code) = 0;
    virtual void setBody(const std::string& body) = 0;
    virtual void setHeader(const std::string& name, const std::string& value) = 0;
    
    // === GETTERS ===
    virtual int getStatus() const = 0;
    virtual std::string getBody() const = 0;
    virtual std::map<std::string, std::string> getHeaders() const = 0;
    virtual std::optional<std::string> getHeader(const std::string& name) const = 0;  // case-insensitive
    
    // === CONVENIENCE ===
    virtual void setResult(int code, const std::string& contentType, const std::string& body) = 0;
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

## 🛣️ Сопоставление маршрутов и Path Parameters

### Паттерны с wildcards

```cpp
registerHandler("GET", "/api/users/123", handler);           // Точное совпадение
registerHandler("GET", "/api/users/*", handler);             // Один path parameter
registerHandler("GET", "/api/orders/*/items/*", handler);    // Несколько параметров
```

### Извлечение Path Parameters

```cpp
void handle(IRequest& req, IResponse& res) override {
    // Паттерн: /api/orders/*/items/*
    // Путь:    /api/orders/ord-123/items/item-456
    
    auto orderId = req.getPathParam(0);  // → "ord-123"
    auto itemId = req.getPathParam(1);   // → "item-456"
    auto missing = req.getPathParam(2);  // → nullopt
    
    // Проверка паттерна
    std::string pattern = req.getPathPattern();  // → "/api/orders/*/items/*"
}
```

### Примеры сопоставления

| Паттерн | Совпадает | Не совпадает |
|---------|-----------|-------------|
| `/api/users` | `/api/users` | `/api/users/123` |
| `/api/users/*` | `/api/users/123` | `/api/users/123/edit` |
| `/api/*/details` | `/api/users/details` | `/api/users/123/details` |
| `/*/orders/*/items/*` | `/v1/orders/123/items/456` | `/orders/123/items` |

---

## 🔑 Работа с заголовками

### Case-insensitive доступ

```cpp
// Все варианты вернут одно значение
auto ct1 = req.getHeader("Content-Type");
auto ct2 = req.getHeader("content-type");
auto ct3 = req.getHeader("CONTENT-TYPE");

// Проверка JSON
if (req.isJson()) {
    auto body = req.getBody();
    // Парсим JSON...
}

// Извлечение Bearer токена
auto token = req.getBearerToken();  // Из "Authorization: Bearer xxx"
```

### Установка заголовков

```cpp
// Один заголовок
req.setHeader("X-Custom", "value");

// Несколько заголовков
req.setHeaders({
    {"X-Request-Id", "123"},
    {"X-Trace-Id", "abc"}
});
```

---

## 📨 HTTP-клиент (Межсервисная коммуникация)

```cpp
#include "HttpClient.hpp"
#include "SimpleRequest.hpp"
#include "SimpleResponse.hpp"

HttpClient client;

// Создаём запрос с новым API
SimpleRequest request;
request.setMethod("POST");
request.setPath("/api/internal/notify");
request.setIp("other-service.local");
request.setPort(8080);
request.setHeader("Authorization", "Bearer TOKEN");
request.setHeader("Content-Type", "application/json");
request.setBody(R"({"event":"user_created"})");

SimpleResponse response;

if (client.send(request, response)) {
    int status = response.getStatus();
    std::string body = response.getBody();
    auto contentType = response.getHeader("Content-Type");
} else {
    std::cerr << "Запрос не удался" << std::endl;
}
```

---

## ⚙️ Конфигурация

### Environment-based конфигурация

```cpp
auto settings = std::make_shared<ServerSettings>(env_);
std::string host = settings->getHost();
int port = settings->getPort();

auto dbSettings = std::make_shared<DbSettings>(env_);
std::string dbHost = dbSettings->getHost();
int dbPort = dbSettings->getPort();
```

### Ручной доступ

```cpp
std::string apiKey = env_->get<std::string>("api.key");
int timeout = env_->get<int>("server.timeout", 30);  // С дефолтом
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

- ✅ **BeastRequestAdapter** — path segments, path parameters, query params, case-insensitive headers, Bearer token, isJson
- ✅ **BeastResponseAdapter** — getters, setResult, case-insensitive getHeader
- ✅ **SimpleRequest** — полная реализация IRequest v2
- ✅ **SimpleResponse** — полная реализация IResponse v2
- ✅ **RouteMatcher** — wildcard matching
- ✅ **ServerSettings / DbSettings** — валидация конфигурации

### Пример теста с новым API

```cpp
#include <gtest/gtest.h>
#include "SimpleRequest.hpp"
#include "SimpleResponse.hpp"

TEST(MyHandler, ExtractsPathParameters) {
    SimpleRequest req("GET", "/api/orders/ord-123/items/item-456", "", "127.0.0.1", 80);
    req.setPathPattern("/api/orders/*/items/*");
    
    SimpleResponse res;
    
    MyHandler handler;
    handler.handle(req, res);
    
    EXPECT_EQ(res.getStatus(), 200);
    EXPECT_EQ(*res.getHeader("Content-Type"), "application/json");
}

TEST(MyHandler, ChecksBearerToken) {
    SimpleRequest req("GET", "/api/protected", "", "127.0.0.1", 80);
    req.setHeader("Authorization", "Bearer valid-token");
    
    auto token = req.getBearerToken();
    ASSERT_TRUE(token.has_value());
    EXPECT_EQ(*token, "valid-token");
}

TEST(MyHandler, UsesAttributes) {
    SimpleRequest req;
    
    req.setAttribute("user_id", "user-123");
    req.setAttribute("role", "admin");
    
    EXPECT_EQ(*req.getAttribute("user_id"), "user-123");
    EXPECT_EQ(*req.getAttribute("role"), "admin");
    EXPECT_FALSE(req.getAttribute("missing").has_value());
}
```

---

## 🏗️ Паттерны проектирования

### 🔌 Паттерн Adapter
`BeastRequestAdapter` и `BeastResponseAdapter` преобразуют объекты Beast в чистые интерфейсы.

### 📋 Паттерн Template Method
`IWebApplication::run()` определяет последовательность запуска:

```cpp
void run(int argc, char* argv[]) {
    loadEnvironment(argc, argv);
    configureInjection();
    start();
}
```

### 🔗 Dependency Injection
Обработчики и настройки регистрируются в `configureInjection()`.

---

## 📚 Структура проекта

```
cpp-http-server/
├── microservice-core/
│   ├── include/
│   │   ├── IRequest.hpp          # Расширенный интерфейс v2
│   │   ├── IResponse.hpp         # С геттерами и setResult()
│   │   ├── IHttpHandler.hpp
│   │   ├── IWebApplication.hpp
│   │   ├── IEnvironment.hpp
│   │   ├── Environment.hpp
│   │   ├── RouteMatcher.hpp
│   │   ├── SimpleRequest.hpp     # Полная реализация v2
│   │   └── SimpleResponse.hpp    # Полная реализация v2
│   ├── src/
│   │   └── RouteMatcher.cpp
│   └── tests/
├── microservice-boost/
│   ├── include/
│   │   ├── BoostBeastApplication.hpp  # HandlerMatch, path parameters
│   │   ├── BeastRequestAdapter.hpp    # Case-insensitive headers
│   │   ├── BeastResponseAdapter.hpp   # С геттерами
│   │   ├── HttpClient.hpp
│   │   └── settings/
│   ├── src/
│   │   └── BoostBeastApplication.cpp
│   └── tests/
├── CMakeLists.txt
├── config.json
├── CHANGELOG.md
└── README.md
```

---

## 🔄 Миграция с v0.0.5

### Изменения API

| Было (v0.0.5) | Стало (v0.1.0) |
|---------------|----------------|
| `getParams()` | `getQueryParams()` (старый метод deprecated) |
| Нет | `getQueryParam(name)` |
| Нет | `getPathParam(index)` |
| Нет | `getHeader(name)` — case-insensitive |
| Нет | `getBearerToken()` |
| Нет | `isJson()` |
| Нет | `setAttribute()` / `getAttribute()` |
| Нет | `setResult(code, contentType, body)` |

### Обратная совместимость

- `getParams()` продолжает работать как alias для `getQueryParams()`
- Все существующие handlers работают без изменений

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
