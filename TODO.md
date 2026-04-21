# cpp-http-server — Backlog

> Задачи разбиты на мелкие, условно-независимые единицы для автоматической обработки ИИ.
> Оценка в Story Points: 1=тривиально, 2=малая, 3=средняя, 5=крупная, 8=очень крупная.
> 
> Источник задач:
> 1. Задачи из TODO.md биржевой платформы (trading-platform), относящиеся к доработке сервера
> 2. Собственные задачи по результату анализа cpp-http-server

---

## v0.3.0 — RELEASED

> Выполнено 36 задач. См. CHANGELOG.md для деталей.
> 
> Выполненные задачи: DX-01, DRY-01, DRY-02, DRY-03, DRY-04, DRY-05, DRY-07, DRY-08,
> SOLID-08, SRV-01, SRV-02, SRV-03, SRV-04, SRV-05, SRV-02b, SRV-06, SRV-07, SRV-08,
> SRV-09, SRV-14, SRV-16, SRV-16a, SRV-16b, SRV-17, SRV-18, SRV-22, SRV-23, SRV-27,
> SRV-29, SRV-30, SRV-37, SRV-38, SRV-39, SRV-40, SRV-41, SRV-42, SRV-43, SRV-06b

---

## v0.4.0 (запланировано)

### SRV-02c: BaseWebApplication — вынести boost-независимую логику
- **SP:** 5
- **Модуль:** microservice-core + microservice-boost
- **Что:** Вынести из BoostBeastApplication в BaseWebApplication (microservice-core): `handlers_`, `findHandler()`, `handleRequest()` (HttpError catch), `registerHandler()` (с проверкой started), `state_`, `logger_`. BoostBeastApplication наследует BaseWebApplication и добавляет только Boost-specific код (io_context, acceptor, handleSession). Это позволяет тестировать роутинг и обработку запросов без Boost-зависимостей.
- **Зачем:** Тестировать `findHandler()`/`handleRequest()` без линковки Boost. Итеративная разработка SRV-11/SRV-13 в microservice-core без пересборки Boost.
- **Зависимости:** Должна быть выполнена **до** SRV-11 и SRV-13 (чтобы не переделывать роутинг дважды)

### SRV-07b: Заголовок Allow в 405 ответе
- **SP:** 1
- **Модуль:** microservice-boost, microservice-core
- **Что:** Добавить заголовок `Allow: GET, POST` в 405 ответ. Требует хранить allowed methods для каждого маршрута и передавать в `HttpErrorSender`.

### SRV-10: Config path через ENV/CLI
- **SP:** 2
- **Модуль:** microservice-boost
- **Что:** `loadEnvironment()` хардкодит `config.json` в текущей директории. Добавить поддержку: (1) ENV переменной `CONFIG_PATH`, (2) CLI аргумента `--config <path>`, (3) fallback на `config.json` в CWD.

### SRV-11: Named path parameters (`:param` syntax)
- **SP:** 5
- **Модуль:** microservice-core
- **Что:** Поддержка `:param` синтаксиса в роутах: `/api/users/:userId`. `getPathParam("orderId")` вместо `getPathParam(0)`.
- **Зависит от:** SRV-02c

### SRV-13: Trie-based routing
- **SP:** 8
- **Модуль:** microservice-core
- **Что:** Заменить O(n) линейный scanning на O(k) trie lookup. Exact match, named parameters, wildcards.
- **Зависит от:** SRV-11

### SRV-19: Health check с dependency checks
- **SP:** 2
- **Модуль:** microservice-core
- **Что:** Расширить HealthHandler: `IHealthCheck` интерфейс с методом `check() -> HealthStatus`. Проверка DB, RabbitMQ зависимости. Аналог Spring Boot Actuator.

### SRV-20: CORS middleware
- **SP:** 2
- **Модуль:** microservice-core
- **Что:** Добавить встроенный `CorsHandler` middleware: OPTIONS preflight → 204 + CORS headers, остальные методы → CORS headers.

### SRV-24: `loadJsonToEnvironment` — arrays skipped
- **SP:** 2
- **Модуль:** microservice-boost
- **Что:** JSON arrays полностью игнорируются. Добавить поддержку: array of strings → comma-separated value.

### SRV-28: Async I/O — переход на асинхронную модель
- **SP:** 13
- **Модуль:** microservice-boost
- **Что:** Заменить синхронную модель на асинхронную на Boost.Asio. Thread pool вместо thread-per-connection. BREAKING CHANGE.

### SRV-31: HTTPS/TLS support
- **SP:** 8
- **Модуль:** microservice-boost
- **Что:** HTTPS сервер: Boost.Asio SSL stream + сертификат/ключ через ServerSettings.

### SRV-32: Async HTTP client
- **SP:** 8
- **Модуль:** microservice-boost
- **Что:** `IAsyncHttpClient` с `sendAsync(request, callback)`. Connection pooling для keep-alive.

### SRV-33: WebSocket support
- **SP:** 5
- **Модуль:** microservice-boost
- **Что:** `IWebSocketHandler` и `WebSocketSession` для real-time уведомлений.

### SRV-35: API Reference документация
- **SP:** 3
- **Модуль:** docs
- **Что:** Разделить README на отдельные документы: docs/api.md, docs/routing.md, docs/middleware.md, docs/deployment.md.

### SRV-36: Миграционный guide с v0.3.0 на v0.4.0
- **SP:** 2
- **Модуль:** docs
- **Что:** docs/migration-v0.4.md с чёткими шагами для каждой breaking change.

### SRV-44: Миграция на UUID v7 (RFC 9562)
- **SP:** 3
- **Модуль:** microservice-core
- **Что:** Текущий `UuidGenerator` не соответствует RFC 4122/RFC 9562. Мигрировать на UUID v7: 48-bit ms timestamp + 74 random bits. `Uuid7Generator : public IIdGenerator`. Старый UuidGenerator оставить как опцию.

### DRY-01b: Генерация main.cpp при сборке (CMake)
- **SP:** 2
- **Модуль:** microservice-boost (CMake)
- **Что:** CMake-функция `add_http_service(TARGET MyService APP_CLASS MyNamespace::MyApp)` генерирует минимальный main.cpp и линкует с microservice-boost.
- **Backward compatible:** старые main.cpp продолжают работать

### DRY-04b: Аналитика — выпиливание nlohmann/json из библиотеки
- **SP:** 3
- **Модуль:** microservice-boost, CMake
- **Что:** Проанализировать возможность удалить nlohmann/json из библиотеки. Consumer сам наполняет IEnvironment, а библиотека не парсит конфиг.

### DRY-06: Аналитика — оценка библиотек для внедрения
- **SP:** 2
- **Модуль:** microservice-core, microservice-boost
- **Что:** Проанализировать: absl::StatusOr, absl::flat_hash_map, boost::lockfree::queue, std::string_view — стоит ли добавлять зависимость.

---

## v0.4.0 — RabbitMQ и Postgres (перенос из trading-platform)

### MSG-01: IEventPublisher + IEventConsumer — интерфейсы сообщений
- **SP:** 3
- **Модуль:** microservice-core
- **Что:** Перенести из trading-platform интерфейсы `IEventPublisher` (publish routingKey, message) и `IEventConsumer` (subscribe routingKeys, handler, start, stop). Следует SOLID/DIP: consumer-проекты зависят от абстракции, не от конкретного RabbitMQ. `InMemoryEventBus` — тестовый double для unit-тестов.
- **Файлы:** Новый `IEventPublisher.hpp`, `IEventConsumer.hpp`, `EventHandler.hpp`, `InMemoryEventBus.hpp`/`.cpp` в microservice-core
- **Тесты:** Unit-тест: InMemoryEventBus publish/subscribe

### MSG-02: RabbitMQAdapter — publisher/consumer + reconnect + connection management
- **SP:** 8
- **Модуль:** microservice-boost
- **Что:** Перенести из trading-platform `RabbitMQAdapter`: publish/subscribe, lifecycle management (Idle→Connecting→Connected→Reconnecting), exponential backoff, pending bindings, thread-safe. Реализует `IEventPublisher` и `IEventConsumer`. Настройки через `RabbitMQSettings` (ENV: RABBITMQ_HOST, PORT, USER, PASSWORD, EXCHANGE, QUEUE_NAME). Зависит от `amqpcpp` + `Boost.Asio`.
- **Файлы:** Новый `RabbitMQAdapter.hpp`/`.cpp`, `RabbitMQSettings.hpp` в microservice-boost
- **Тесты:** Интеграционный тест с Testcontainers RabbitMQ (или InMemoryEventBus для unit)
- **Связанные задачи:** trading-platform REL-02, REL-08, MET-01, MET-03

### MSG-03: Circuit breaker для исходящих вызовов
- **SP:** 5
- **Модуль:** microservice-core
- **Что:** Перенести из trading-platform REL-05: `CircuitBreaker` (CLOSED/OPEN/HALF_OPEN). Порог: N ошибок за T секунд → OPEN, half-open после timeout. Оборачивает `IHttpClient` и `IEventPublisher`. Следует SOLID: `ICircuitBreaker` интерфейс.
- **Файлы:** Новый `ICircuitBreaker.hpp`, `CircuitBreaker.hpp`/`.cpp` в microservice-core
- **Тесты:** Unit-тест: state transitions, threshold → open, timeout → half-open, success → closed

### MSG-04: Retry policy с exponential backoff
- **SP:** 3
- **Модуль:** microservice-core
- **Что:** Перенести из trading-platform REL-06: `RetryPolicy` для HTTP и RabbitMQ. Retry на 5xx и network errors. Max 3 attempts, backoff 1s/2s/4s. Настраиваемая стратегия.
- **Файлы:** Новый `IRetryPolicy.hpp`, `RetryPolicy.hpp`/`.cpp` в microservice-core
- **Тесты:** Unit-тест: retry on 5xx, max attempts, backoff timing

### MSG-05: RabbitMQ metrics (published, received, errors)
- **SP:** 3
- **Модуль:** microservice-boost
- **Что:** Перенести из trading-platform MET-03: счётчики в `RabbitMQAdapter` — `amqp_published_total`, `amqp_received_total`, `amqp_errors_total`. Интеграция с `IMetricsCollector`.
- **Файлы:** `RabbitMQAdapter.hpp`/`.cpp` (добавить metrics)
- **Связанные задачи:** SRV-30 (IMetricsCollector уже реализован)

### DB-01: IConnectionPool + ConnectionPool — Postgres connection pool
- **SP:** 5
- **Модуль:** microservice-boost (или отдельный cpp-postgres-pool)
- **Что:** Перенести из trading-platform `ConnectionPool`: thread-safe PostgreSQL connection pool. Pre-creates minConnections, grows to maxConnections. RAII `PooledConnection` возвращает соединение автоматически. Проверка `is_open()` при checkout, пересоздание мёртвых соединений. `shutdown()` для graceful termination. Реализует `IShutdown` (интеграция с ShutdownManager).
- **Файлы:** Новый `IConnectionPool.hpp` в microservice-core, `PostgresConnectionPool.hpp`/`.cpp` в microservice-boost (или отдельный модуль). Зависит от `libpqxx`.
- **Тесты:** Integration test с Testcontainers PostgreSQL
- **Связанные задачи:** SRV-08 (IShutdown + ShutdownManager уже реализован)

### DB-02: ITransactionExecutor + PostgresTransactionExecutor
- **SP:** 3
- **Модуль:** microservice-boost
- **Что:** Перенести из trading-platform `TransactionExecutor`: `query<T>()` для reads, `execute()` для writes. Wraps `pqxx::work` в try/commit/catch, логирует ошибки, rethrow.
- **Файлы:** Новый `ITransactionExecutor.hpp` в microservice-core, `TransactionExecutor.hpp`/`.cpp` в microservice-boost
- **Тесты:** Integration test с PostgreSQL

### DB-03: Health check для Postgres и RabbitMQ
- **SP:** 2
- **Модуль:** microservice-core
- **Что:** Реализация `IHealthCheck` для Postgres (`SELECT 1`) и RabbitMQ (`connection.isOpen()`). Регистрируются в HealthHandler через DI.
- **Файлы:** `PostgresHealthCheck.hpp`, `RabbitMQHealthCheck.hpp`
- **Связанные задачи:** SRV-19 (IHealthCheck — будет реализован в v0.4.0)

---

## Сводка по Story Points

| Категория | Задач | SP |
|-----------|-------|-----|
| v0.4.0 Architecture | 3 | 10 |
| v0.4.0 Features | 6 | 19 |
| v0.4.0 Docs & DX | 2 | 4 |
| v0.4.0 DRY | 2 | 5 |
| v0.4.0 RabbitMQ | 5 | 22 |
| v0.4.0 Postgres | 3 | 10 |
| **Итого** | **21** | **70** |