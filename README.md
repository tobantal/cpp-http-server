# cpp-http-server

Современная библиотека HTTP-сервера на C++17 с чистой архитектурой (hexagonal/ports-and-adapters).

#### Автор: Тоболкин Антон

---

## Архитектура

Библиотека состоит из **двух модулей**:

### Core Module (`microservice-core`)

Чистые абстракции и утилиты — **нулевые зависимости** (header-only + RouteMatcher.cpp).

| Компонент | Назначение |
|-----------|-----------|
| `IRequest` / `IResponse` | HTTP-запрос и ответ: path params, query params, headers, body, trace ID |
| `IHttpHandler` | Обработчик маршрутов с `name()` для логирования |
| `IWebApplication` | Template Method: `run()` → signal handling, loadEnvironment, configureInjection, start |
| `IShutdown` / `ShutdownManager` | Graceful shutdown в LIFO-порядке с timeout |
| `IHttpErrorHandler` / `HttpErrorSender` | Обработка ошибок: `handleError(res, HttpError)` |
| `IMetricsCollector` / `MetricsCollector` | Counter, gauge, histogram → Prometheus format |
| `MetricsObserverHandler` | Декоратор: обёртка над handler chain, записывает http_requests_total + duration |
| `MetricsHandler` | `GET /metrics` — Prometheus text format |
| `ChainHandler` | Middleware chain с IHttpErrorHandler injection |
| `RouteMatcher` | Сопоставление маршрутов с wildcards |
| `IIdGenerator` / `UuidGenerator` | Thread-safe генератор ID (DIP: mockable в тестах) |
| `Uuid7Generator` | UUID v7 (RFC 9562) — time-ordered, k-sortable IDs |
| `IEnvironment` / `Environment` | Type-safe конфигурация (ENV → config.json → default) |
| `Schema` / `FieldDef` / `FieldBuilder` | Fluent DSL for request schema definitions |
| `ISchemaValidator` | Port for schema validation (inherits `IHttpHandler`) |
| `IHttpClient` | HTTP-клиент для межсервисной коммуникации |
| `ICircuitBreaker` | Circuit breaker pattern: CLOSED/OPEN/HALF_OPEN states, threshold-based transition |
| `CircuitBreaker` | Implementation: failure counting, timeout-based half-open transition |
| `IRetryPolicy` | Retry strategy interface: `shouldRetry()`, `getDelay()` |
| `RetrySettings` | Configuration: maxAttempts, initialDelay, maxDelay, backoffMultiplier |
| `HttpRetryExecutor` | Decorator for IHttpClient: wraps calls with retry logic |
| `IEventPublisher` | Публикация событий: `publish(routingKey, message)` |
| `IEventConsumer` | Подписка на события: `subscribe(routingKeys, handler)`, `start()`, `stop()` |
| `InMemoryEventBus` | Test double: IEventPublisher + IEventConsumer (ExceptionPolicy, message recording) |
| `IConnectionPool` | Connection pool lifecycle: `available()`, `size()`, `isAlive()`, extends IShutdown |
| `ITransactionExecutor` | Marker interface для DI wiring транзакций |
| `IDbSettings` | Database settings: host, port, name, user, password, pool sizing, connectionString |
| `IRepository<T>` | Generic CRUD interface: findById, findAll, save, saveAll, removeById |
| `KeyValueEntity` | Simple key-value entity struct (id + value) |
| `InMemoryKeyValueRepository` | Thread-safe in-memory IRepository<KeyValueEntity> for tests and caching |
| `Timer` | Утилита замера времени (start/stop/elapsed/show) |
| `HttpLogger` / `SplunkLogger` | HTTP-based logging with IShutdown graceful flush, Splunk auth, fallback to NullLogger |
| `SimpleRequest` / `SimpleResponse` | Test doubles для IRequest/IResponse |

### Boost Module (`microservice-boost`)

Production-ready HTTP-сервер на Boost.Beast/Asio.

| Компонент | Назначение |
|-----------|-----------|
| `BoostBeastApplication` | HTTP-сервер: keep-alive, connection limits, timeouts, version header |
| `BeastRequestAdapter` / `BeastResponseAdapter` | Адаптеры Beast → IRequest/IResponse |
| `HttpClient` | Исходящий HTTP-клиент (connect/read/write timeout) |
| `RetryingHttpClient` | HttpClient wrapper with RetryPolicy: exponential backoff on failures |
| `RabbitMQAdapter` | RabbitMQ: IEventPublisher + IEventConsumer + IShutdown, lifecycle state machine, reconnect with exponential backoff |
| `RabbitMQSettings` | Конфигурация RabbitMQ: ENV → config.json → default |
| `ConnectionPool` | Thread-safe PostgreSQL connection pool: PooledConnection RAII, min/max sizing, blocking checkout |
| `PostgresTransactionExecutor` | Template query/execute wrapper for pqxx::work with error logging |
| `DbSettings` | Конфигурация БД: ENV → config.json → default (prefix support) |
| `DatabaseHealthHandler` | /health/db endpoint: isAlive, available, size → JSON |
| `PostgresKeyValueRepository` | Example IRepository<KeyValueEntity> with ensureSchema(), upsert, parameterized queries |
| `SchemaValidator` | Validates IEnvironment fields against Schema: required, type, min/max, minLength/maxLength → BadRequestError(400) |
| `ServerSettings` | Конфигурация сервера: ENV → config.json → default |

---

## Установка

### CMake FetchContent

```cmake
cmake_minimum_required(VERSION 3.14)
project(my_service)

include(FetchContent)

FetchContent_Declare(
    cpp-http-server
    GIT_REPOSITORY https://github.com/tobantal/cpp-http-server.git
    GIT_TAG v0.5.0
)

# Если ваш проект уже подтягивает Boost/nlohmann_json:
# set(CPP_HTTP_SERVER_FETCH_DEPS OFF)

FetchContent_MakeAvailable(cpp-http-server)

add_executable(my_app src/main.cpp)
target_link_libraries(my_app microservice-boost)
```

### Требования

- **C++17** или выше
- **CMake 3.14+**
- **Boost 1.70+** (Asio, Beast, System)
- **nlohmann/json** 3.9+ (для config.json в microservice-boost)
- **amqpcpp** 4.3+ (для RabbitMQ в microservice-boost, FetchContent auto-fetch)
- **libpqxx** 7.0+ (для PostgreSQL в microservice-boost, FetchContent auto-fetch)

### Сборка и тесты

```bash
git clone https://github.com/tobantal/cpp-http-server.git
cd cpp-http-server
cmake -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build
cd build && ctest --verbose
```

---

## Быстрый старт

### 1. Приложение

```cpp
#include "BoostBeastApplication.hpp"
#include "ChainHandler.hpp"
#include "HealthHandler.hpp"

class MyWebApp : public BoostBeastApplication {
public:
    void configureInjection() override {
        registerEndpoint("GET", "/status", std::make_shared<HealthHandler>());
        registerEndpoint("GET", "/api/users", std::make_shared<GetUsersHandler>());
    }
};
```

### 2. main.cpp — 3 строки (signal handling встроен)

```cpp
#include "MyWebApp.hpp"

int main(int argc, char* argv[]) {
    MyWebApp app;
    return app.run(argc, argv);
}
```

Signal handling (SIGINT/SIGTERM) встроен в `IWebApplication::run()`. Try/catch тоже внутри.

### 3. Конфигурация

**config.json:**
```json
{
  "server": {
    "host": "0.0.0.0",
    "port": 8080,
    "maxRequestBodySize": 1048576,
    "readTimeoutMs": 30000,
    "writeTimeoutMs": 30000,
    "maxConnections": 0,
    "maxRequestsPerConnection": 100
  }
}
```

См. [docs/configuration.md](docs/configuration.md) для полного списка настроек и [.env.example](.env.example) для ENV-переменных.

---

## Ключевые возможности v0.4.0

### Messaging (RabbitMQ)

```cpp
auto settings = std::make_shared<RabbitMQSettings>(env);
auto rabbitMQ = std::make_shared<RabbitMQAdapter>(settings, logger, metrics);

rabbitMQ->subscribe({"order.created", "order.cancelled"},
    [](const std::string& routingKey, const std::string& message) {
        // handle event
    });

rabbitMQ->start();  // Idle → Connecting → Connected

rabbitMQ->publish("order.created", R"({"id":"ord-1"})");

shutdownMgr->registerComponent(rabbitMQ);  // implements IShutdown
```

Lifecycle: `Idle → Connecting → Connected ↔ Reconnecting` with exponential backoff (1s→30s).

Metrics: `amqp_published_total`, `amqp_received_total`, `amqp_errors_total`.

### Database (PostgreSQL)

```cpp
auto dbSettings = std::make_shared<DbSettings>(env);
auto pool = std::make_shared<ConnectionPool>(
    dbSettings->getConnectionString(),
    ConnectionPool::Config{dbSettings->getMinConnections(), dbSettings->getMaxConnections()},
    logger);

shutdownMgr->registerComponent(pool);  // implements IShutdown

// Health check endpoint
registerEndpoint("GET", "/health/db",
    std::make_shared<DatabaseHealthHandler>(pool));

// Repository pattern
auto repo = std::make_shared<PostgresKeyValueRepository>(pool, logger);
repo->ensureSchema();
repo->save({"user:1", R"({"name":"Alice"})"});
auto entity = repo->findById("user:1");
```

Connection pool: thread-safe, PooledConnection RAII, min/max sizing, blocking checkout.
Repository: generic `IRepository<T>` interface, parameterized queries, upsert support.

### Graceful Shutdown

```cpp
auto app = std::make_shared<MyWebApp>();
auto shutdownMgr = std::make_shared<ShutdownManager>(logger);
shutdownMgr->registerComponent(app);  // app implements IShutdown
// SIGINT/SIGTERM → ShutdownManager::shutdownAll() в LIFO-порядке
```

### Metrics (Prometheus)

```cpp
auto metrics = std::make_shared<MetricsCollector>();
auto observer = std::make_shared<MetricsObserverHandler>(chainHandler, metrics, "my-service");
// observer оборачивает handler chain, записывает http_requests_total + duration

auto metricsHandler = std::make_shared<MetricsHandler>(metrics);
registerEndpoint("GET", "/metrics", metricsHandler);
```

### Error Handling (IHttpErrorHandler)

```cpp
// HttpErrorSender — default, JSON error responses
// Можно внедрить свой формат:
class XmlErrorHandler : public IHttpErrorHandler {
    void handleError(IResponse& res, const HttpError& e) override {
        res.setResult(e.statusCode(), "application/xml",
            "<error>" + e.message() + "</error>");
    }
};
```

### Schema Validation (SchemaValidator)

```cpp
// 1. Define schema per endpoint (subclass Schema)
class OrderSchema : public Schema {
public:
    OrderSchema() {
        field<std::string>("symbol").required().minLength(1).maxLength(10);
        field<int>("amount").required().min(1);
        field<double>("price").optional().max(1000000);
    }
};

// 2. Wire into ChainHandler (after JsonProcessor, before business handler)
auto orderValidator = std::make_shared<SchemaValidator>(
    std::make_shared<OrderSchema>());

registerEndpoint("POST", "/orders",
    ChainHandler(logger, errorHandler,
        jsonProcessor,
        orderValidator,    // validates required fields, types, constraints
        createOrderHandler // guaranteed valid data — no try/catch needed
    ));
```

### Trace ID + Handler Names in Logs

```
[trace-abc123] HealthHandler started
[trace-abc123] HealthHandler finished (2ms) with status 200
[trace-abc123] ChainHandler started
```

### Keep-Alive

Множественные запросы на одном TCP-соединении (до `SERVER_MAX_REQUESTS_PER_CONNECTION=100`).

### Connection Limits

- `SERVER_MAX_CONNECTIONS=0` — unlimited, при превышении → 503
- `SERVER_MAX_REQUEST_BODY_SIZE=1048576` — при превышении → 413
- Timeouts: `SERVER_READ_TIMEOUT_MS`, `SERVER_WRITE_TIMEOUT_MS`

---

## SOLID принципы

- **S** — один класс = одна ответственность (ChainHandler = middleware, MetricsObserverHandler = metrics, etc.)
- **O** — расширение через интерфейсы: `IHttpHandler`, `IMetricsCollector`, `IHttpErrorHandler`, `IShutdown`, `IIdGenerator`
- **L** — реализации взаимозаменяемы: `UuidGenerator` ↔ deterministic mock в тестах
- **I** — `INameable` выделен отдельно от `IHttpHandler` и `IShutdown`
- **D** — зависимости от абстракций: `ChainHandler` зависит от `IHttpErrorHandler`, не от `HttpErrorSender`

---

## Структура проекта

```
cpp-http-server/
├── microservice-core/               # Нулевые зависимости
│   ├── include/
│   │   ├── domain/                 # Доменные типы: HttpError, INameable, IResponse, Schema
│   │   ├── schema/                 # Schema, FieldDef, FieldBuilder
│   │   ├── error/                   # NotFoundError, BadRequestError, MethodNotAllowedError
│   │   ├── ports/input/             # IHttpHandler, IHttpErrorHandler, IWebApplication
│   │   ├── ports/output/            # ILogger, IShutdown, IEnvironment, IMetricsCollector, IEventPublisher, IEventConsumer, IConnectionPool, ITransactionExecutor
│   │   ├── application/            # ChainHandler, ShutdownManager
│   │   ├── handler/                # HealthHandler, MetricsHandler, MetricsObserverHandler, HttpErrorSender
│   │   ├── metrics/                # MetricsCollector, PrometheusSerializer
│   │   ├── messaging/              # EventHandler, ExceptionPolicy, PublishedMessage
│   │   ├── repository/             # IRepository<T>, KeyValueEntity
│   │   ├── util/                   # StringUtils, Timer, IIdGenerator, UuidGenerator, PathParamExtractor
│   │   ├── settings/               # IServerSettings, IDbSettings, Environment
│   │   └── adapters/secondary/    # SimpleRequest, SimpleResponse, NullLogger, TestLogger
│   └── src/
├── microservice-boost/             # Boost.Beast + Asio
│   ├── include/
│   │   ├── BoostBeastApplication.hpp
│   │   ├── BeastRequestAdapter.hpp
│   │   ├── BeastResponseAdapter.hpp
│   │   ├── HttpClient.hpp
│   │   ├── RabbitMQAdapter.hpp
│   │   ├── ReconnectBoostAsioHandler.hpp
│   │   ├── adapters/primary/       # DatabaseHealthHandler, SchemaValidator
│   │   ├── adapters/secondary/    # ConnectionPool, PostgresTransactionExecutor, SplunkLogger
│   │   ├── settings/ServerSettings.hpp
│   │   ├── settings/RabbitMQSettings.hpp
│   │   ├── settings/DbSettings.hpp
│   │   ├── messaging/RabbitMQConnectionState.hpp
│   │   ├── repository/             # PostgresKeyValueRepository, KeyValueEntity
│   └── src/
├── docs/
│   ├── configuration.md
│   └── ci-extended-configuration.md
├── .env.example
├── CMakeLists.txt
├── CHANGELOG.md
└── README.md
```

---

## Конфигурация

См. [docs/configuration.md](docs/configuration.md) для полной таблицы настроек.

Приоритет: **ENV** > **config.json** > **default**.

---

## Покрытие тестами

448+ тестов (core: 341, boost: 174+). Покрытие по модулям:

- **Core:** ChainHandler, RouteMatcher, Environment, HttpError, MetricsCollector, MetricsObserverHandler, MetricsHandler, HttpErrorSender, ShutdownManager, Timer, UuidGenerator, InMemoryEventBus, InMemoryKeyValueRepository, Version
- **Boost:** BeastRequestAdapter (path params, query params, headers, trace ID, keep-alive), BeastResponseAdapter, ServerSettings, HttpClient, ConnectionPool (RAII, thread safety, schema), PostgresTransactionExecutor, DbSettings

---

## Лицензия

MIT License. См. [LICENSE](LICENSE) для деталей.