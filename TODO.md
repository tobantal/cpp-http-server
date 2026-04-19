# cpp-http-server — Backlog

> Задачи разбиты на мелкие, условно-независимые единицы для автоматической обработки ИИ.
> Оценка в Story Points: 1=тривиально, 2=малая, 3=средняя, 5=крупная, 8=очень крупная.
> 
> Источник задач:
> 1. Задачи из TODO.md биржевой платформы (trading-platform), относящиеся к доработке сервера
> 2. Собственные задачи по результату анализа cpp-http-server
> 
> Конечная цель библиотеки — максимально упростить, улучшить код биржевой торговли, сделать его читабельным, удобным для разворачивания, надёжным, безопасным, быстрым.

---

## P1 — DRY (убрать дублирование из consumer-проектов)

### DRY-01: Исследование — включить main.cpp в поставку библиотеки
- **SP:** 2
- **Модуль:** microservice-boost
- **Что:** Проанализировать возможность включить `main()` в поставку cpp-http-server, чтобы consumer-проекты (trading-platform) не дублировали его в каждом микросервисе. Сейчас `main.cpp` во всех трёх сервисах практически идентичен: signal handling (SIGINT/SIGTERM → app.stop()), `app.run(argc, argv)`, try/catch. Signal handling включить в библиотечный main. Варианты поставки: (а) готовый `main()`, вызывающий пользовательский `createApp()` — пользователь только определяет класс App; (б) макрос `DEFINE_APPLICATION(MyApp)` — разворачивается в main + boilerplate; (в) функция-обёртка `runApplication(argc, argv, factory)` — пользователь передаёт фабрику. Исследовать: совпадает ли signal handling, какие различия между сервисами, какой вариант наиболее гибкий и не ломает backward compatibility. Результат — решение + задача на реализацию.
- **Файлы:** Анализ: `trading-platform/education/*/src/main.cpp` (3 файла), `BoostBeastApplication.hpp`
- **Результат:** Документ с анализом + рекомендация по варианту (пока документируем, реализация — отдельная задача)

### ~~DRY-03~~ ✅ ВЫПОЛНЕНО — см. CHANGELOG

### DRY-04b: Аналитика — выпиливание nlohmann/json и config.json
- **SP:** 3
- **Модуль:** microservice-boost, CMake
- **Что:** Проанализировать возможность удалить nlohmann/json и config.json из библиотеки. Сейчас `loadEnvironment()` читает `config.json` через nlohmann/json и вызывает `loadJsonToEnvironment()`. Альтернатива: consumer сам наполняет IEnvironment, а библиотека не парсит конфиг вообще. Но config.json может хранить настройки, которые неудобно передавать через ENV (имя сервера, настройки логирования в будущем). Возможный компромисс: оставить nlohmann/json + config.json для backward compatibility, но парсить значения в ENV через `setenv()` — тогда consumer может читать их через `std::getenv()` единообразно. Результат — документ с рекомендацией.
- **Файлы:** Анализ: `BoostBeastApplication.cpp` (loadJsonToEnvironment), `CMakeLists.txt`
- **Результат:** Документ с анализом + рекомендация (пока документируем, реализация — отдельная задача)
- **Связанные задачи:** DRY-03 (тощая поставка — после выпиливания json задача упрощается)

### DRY-06: Аналитика — оценка библиотек для возможного внедрения
- **SP:** 2
- **Модуль:** microservice-core, microservice-boost
- **Что:** Проанализировать библиотеки из LIBRARIES.md trading-platform на применимость к cpp-http-server. Фокус на тех, что напрямую улучшают библиотеку сервера. Кандидаты для анализа: (а) **absl::StatusOr<T>** — замена `bool` return в IHttpClient::send(), замена `std::optional` в error paths, более информативный паттерн ошибок; (б) **absl::flat_hash_map** — замена `std::map` для handlers_ (быстрее lookup для роутинга); (в) **boost::lockfree::queue** — для thread-safe очереди задач при переходе на thread pool (SRV-03/SRV-04); (г) **string_view** (C++17, уже доступен) — замена `const std::string&` в IRequest/IResponse методах, устранение лишних аллокаций. Отдельный вопрос: стоит ли добавлять зависимость от Abseil ради StatusOr/flat_hash_map, или проще реализовать аналог своими силами. Результат — документ с рекомендацией по каждой библиотеке.
- **Результат:** Документ с анализом + рекомендации (пока документируем, реализация — отдельная задача)

### ~~DRY-07~~ ✅ ВЫПОЛНЕНО — см. CHANGELOG

### ~~DRY-08~~ ✅ ВЫПОЛНЕНО — см. CHANGELOG

---

## P0 — Critical (блокирует производство)

### SRV-02c: Промежуточная абстракция BaseWebApplication
- **SP:** 5
- **Модуль:** microservice-core
- **Что:** Вынести boost-независимую логику из BoostBeastApplication в BaseWebApplication: `handlers_`, `findHandler()`, `handleRequest()` (HttpError catch), `registerHandler()` (с проверкой started), `started_` флаг. BoostBeastApplication наследует BaseWebApplication и добавляет только Boost-specific код (io_context, acceptor, handleSession). Это позволит в будущем создать вторую реализацию на другой HTTP-библиотеке без дублирования routing/error-handling логики.

### ~~SRV-04~~ ✅ ВЫПОЛНЕНО — см. CHANGELOG

---

## P1 — Security & Reliability

### SRV-07b: Заголовок Allow в 405 ответе
- **SP:** 1
- **Модуль:** microservice-boost
- **Что:** Добавить заголовок `Allow: GET, POST` в 405 ответ. Требует хранить allowed methods для каждого маршрута и передавать в MethodNotAllowedError. Низкий приоритет — внутренний API не использует Allow.
- **Файлы:** `MethodNotAllowedError.hpp`, `BoostBeastApplication.cpp`

### SRV-08: Graceful shutdown (IShutdown + ShutdownManager)
- **SP:** 5
- **Модуль:** microservice-core + microservice-boost
- **Что:** При SIGTERM/SIGINT: прекратить приём новых соединений, доработать текущие запросы (с таймаутом), закрыть все подсистемы в обратном порядке. Решение:
  1. **`IShutdown` интерфейс** (microservice-core): `virtual void shutdown() = 0` + виртуальный деструктор. Все подсистемы, требующие graceful shutdown (HTTP-сервер, connection pool, background workers), реализуют этот интерфейс.
  2. **`ShutdownManager`** (microservice-core): регистрирует `IShutdown`-объекты в порядке старта. При shutdown вызывает `shutdown()` в обратном порядке (LIFO). Таймаут на каждый `shutdown()` (default: 5s). Если таймаут истёк — логировать warning и продолжить.
  3. **`BoostBeastApplication`**: реализует `IShutdown`. `shutdown()` = close acceptor + drain current requests + join threads + stop io_context.
  4. **Signal handler**: SIGTERM/SIGINT → `ShutdownManager::shutdownAll()`. Wire в `run()` или `start()`.
  5. Consumer-проекты регистрируют свои подсистемы (RabbitMQAdapter, ConnectionPool, BackgroundTicker) в тот же ShutdownManager — единая точка graceful shutdown для всего приложения.
- **Файлы:** Новый `IShutdown.hpp`, `ShutdownManager.hpp` в microservice-core; `BoostBeastApplication.hpp`/`.cpp` (реализует IShutdown, signal handler)
- **Тесты:** Unit-тест: ShutdownManager LIFO order + timeout; Integration-тест: SIGTERM → graceful shutdown → текущий запрос завершается
- **Ссылка:** trading-platform REL-08 (ICloseable + ShutdownManager — будет использовать IShutdown из библиотеки)

### ~~SRV-09~~ ✅ ВЫПОЛНЕНО — см. CHANGELOG

### SRV-10: Config path через ENV/CLI
- **SP:** 2
- **Модуль:** microservice-boost
- **Что:** `loadEnvironment()` хардкодит `config.json` в текущей директории. Добавить поддержку: (1) ENV переменной `CONFIG_PATH`, (2) CLI аргумента `--config <path>`, (3) fallback на `config.json` в CWD. В trading-platform сервисы читают конфиг из K8s ConfigMap — путь может быть `/etc/app/config.json`.
- **Файлы:** `BoostBeastApplication.cpp`
- **Тесты:** Unit-тест: ENV variable → correct config path; CLI arg → correct config path; missing config → clear error

---

## P1 — API Improvements (из trading-platform)

### SRV-11: Named path parameters (`:param` syntax) — FUT-02
- **SP:** 5
- **Модуль:** microservice-core
- **Что:** Поддержка `:param` синтаксиса в роутах: `/api/users/:userId`, `/api/orders/:orderId/items/:itemId`. `getPathParam("orderId")` вместо `getPathParam(0)`. Сохранить backward compatibility с `*` wildcard (позиционный доступ). Обновить RouteMatcher для named groups. В trading-platform: `/api/v1/accounts/*` → `/api/v1/accounts/:accountId`.
- **Файлы:** `RouteMatcher.hpp`, `RouteMatcher.cpp`, `IRequest.hpp`, `SimpleRequest.hpp`, `BeastRequestAdapter.hpp`, `BoostBeastApplication.cpp`
- **Тесты:** Unit-тест: named params extraction, mixed wildcards + named params, backward compatibility с `*`
- **Ссылка:** trading-platform FUT-02, API-00

### SRV-13: Trie-based routing — FUT-01
- **SP:** 8
- **Модуль:** microservice-core
- **Что:** Заменить O(n) линейный scanning по wildcard-паттернам на O(k) trie lookup, где k = длина URL. Поддержка exact match, named parameters (`:id`), wildcards (`*`). Радикальное ускорение для 33+ endpoint-ов (в trading-platform), и масштабируемость для 100+.
- **Файлы:** Новый `TrieRouter.hpp`/`.cpp` в microservice-core, `BoostBeastApplication.cpp` (замена findHandler)
- **Тесты:** Benchmark: trie vs linear для 33, 100, 500 маршрутов; Unit-тесты: exact, named, wildcard, priority (most specific match)
- **Зависит от:** SRV-11 (named params — сначала определить API)

### ~~SRV-14~~ ✅ ВЫПОЛНЕНО — см. CHANGELOG

---

## P2 — Observability & Developer Experience

### ~~SRV-16~~ ✅ ПОГЛОЩЕНО SRV-16a — см. CHANGELOG

### ~~SRV-16a~~ ✅ ВЫПОЛНЕНО — см. CHANGELOG

### ~~SRV-16b~~ ✅ ВЫПОЛНЕНО — см. CHANGELOG

### ~~SRV-17~~ ✅ ВЫПОЛНЕНО — см. CHANGELOG

### SRV-18: Tracing middleware — X-Trace-ID в ChainHandler
- **SP:** 3
- **Модуль:** microservice-core + microservice-boost
- **Что:** Встроить поддержку X-Trace-ID в библиотеку. Архитектура (обсуждено):
  1. `IRequest::getTraceId()` — извлечь `X-Trace-ID` из заголовка или сгенерировать UUID v4 (lazy, кешируется через `setAttribute/getAttribute`)
  2. `IRequest::setTraceId(id)` — ручная установка (редко нужно)
  3. `IResponse::setTraceId(id)` — установить `X-Trace-ID` в ответ
  4. `StringUtils::generateUuid()` — UUID v4 (copy-paste из trading-platform, без внешних зависимостей)
  5. **ChainHandler** автоматически: (а) извлекает/генерирует trace ID, (б) прокидывает `setTraceId` в ответ, (в) включает trace ID в лог ошибок — отдельный TraceHandler middleware НЕ нужен
  6. Обработчики получают trace ID через `req.getTraceId()` — для исходящих HTTP-вызовов
  7. HttpClient::send() НЕ пробрасывает trace ID автоматически (явное лучше неявного)
- **Файлы:** `IRequest.hpp`, `IResponse.hpp`, `SimpleRequest.hpp`, `SimpleResponse.hpp`, `BeastRequestAdapter.hpp`, `BeastResponseAdapter.hpp`, `ChainHandler.hpp`/`.cpp`, `StringUtils.hpp`/`.cpp`
- **Тесты:** Unit-тест: request с X-Trace-ID → response с тем же ID; request без → response с новым UUID; ChainHandler автоматически пробрасывает
- **Зависит от:** SRV-16b (рефакторинг логера — конструкторная инъекция, после неё ChainHandler будет логировать через logger_ без null-проверок)
- **Ссылка:** trading-platform OBS-02

### SRV-19: Health check с dependency checks
- **SP:** 2
- **Модуль:** microservice-core
- **Что:** Расширить HealthHandler: поддержка проверок зависимостей (DB, RabbitMQ). `HealthHandler` принимает `std::vector<IHealthCheck>` — интерфейс с методом `check() -> HealthStatus`. В trading-platform REL-07 требует `/health` на каждом сервисе.
- **Файлы:** `HealthHandler.hpp`, новый `IHealthCheck.hpp`
- **Тесты:** Unit-тест: HealthHandler с mock health checks → JSON response с detailed status
- **Ссылка:** trading-platform REL-07

### SRV-20: CORS middleware
- **SP:** 2
- **Модуль:** microservice-core
- **Что:** Browser-based API клиенты требуют CORS headers. Добавить встроенный `CorsHandler` middleware: OPTIONS preflight → 204 + CORS headers, остальные методы → CORS headers в ответе. Настройки: `allowedOrigins`, `allowedMethods`, `allowedHeaders`.
- **Файлы:** Новый `CorsHandler.hpp` в microservice-core
- **Тесты:** Unit-тест: OPTIONS preflight → 204 + CORS; GET с Origin → CORS headers

---

## P1 — API Improvements (from trading-platform: REF-11)


## P2 — Code Quality & Bug Fixes

### SRV-23: `getQueryParams()` парсинг при каждом вызове
- **SP:** 1
- **Модуль:** microservice-boost
- **Что:** `BeastRequestAdapter::getQueryParams()` перепарсит URL при каждом вызове. Кэшировать результат в `mutable std::optional<QueryParams>` с lazy initialization.
- **Файлы:** `BeastRequestAdapter.hpp`
- **Тесты:** Unit-тест: multiple calls → parsed once

### SRV-24: `loadJsonToEnvironment` — arrays skipped
- **SP:** 2
- **Модуль:** microservice-boost
- **Что:** JSON arrays полностью игнорируются при загрузке конфига. Добавить поддержку: array of strings → comma-separated value, или array of objects → недопустимо (логировать warning). Пример: `"allowedOrigins": ["http://localhost:3000", "http://localhost:8080"]` → `allowedOrigins = "http://localhost:3000,http://localhost:8080"`.
- **Файлы:** `BoostBeastApplication.cpp`
- **Тесты:** Unit-тест: JSON с array → parsed корректно

### ~~SRV-25~~ ✅ ПОГЛОЩЕНО SRV-09 — см. CHANGELOG

### ~~SRV-27~~ ✅ ВЫПОЛНЕНО — см. CHANGELOG

---

## P3 — Performance & Future

### SRV-28: Async I/O — переход на асинхронную модель
- **SP:** 13
- **Модуль:** microservice-boost
- **Что:** Заменить синхронную модель (один поток на соединение) на асинхронную на Boost.Asio. `io_context.run()` с thread pool для handlers, async read/write через `async_read_some()` + `async_write()`. Это масштабируемое решение: 10K+ соединений на одном процессе. Включает: (1) фиксированный thread pool вместо создания thread на запрос; (2) очередь запросов с bounded capacity; (3) async обработку соединений. BREAKING CHANGE — новая архитектура, но интерфейс IRequest/IResponse остаётся.
- **Файлы:** `BoostBeastApplication.hpp`/`.cpp` — полная переработка, новый `ThreadPool.hpp`
- **Тесты:** Integration-тесты: concurrent connections, load testing (wrk/hey/ab), graceful shutdown
- **Зависит от:** SRV-04 (connection limit уже реализован, thread pool добавляется здесь)

### SRV-29: Keep-alive support
- **SP:** 3
- **Модуль:** microservice-boost
- **Что:** Текущая модель: одно соединение → один запрос → закрыть. boost.Beast поддерживает keep-alive через `res.keep_alive(req.keep_alive())`, но `handleSession` не реализует persist-connection loop. Добавить: read request → process → write response → если keep_alive → читать следующий request. Лимит: maxRequestsPerConnection (default: 100).
- **Файлы:** `BoostBeastApplication.cpp`
- **Тесты:** Integration-тест: keep-alive → несколько запросов на одном соединении; maxRequests → close

### SRV-30: MetricsHandler — Prometheus endpoint
- **SP:** 3
- **Модуль:** microservice-core
- **Что:** `MetricsHandler` в CHANGELOG.md v0.3.0, но не реализован. Добавить `MetricsHandler : IHttpHandler` — собирает базовые HTTP метрики (requests total, latency histogram) и отдаёт в Prometheus формате. Интерфейс `IMetricsCollector` для инъекции из consumer-проекта. В trading-platform MET-01 нужен `http_requests_total` с label status_code — сейчас MetricsMiddleware не может получить response status.
- **Файлы:** Новый `MetricsHandler.hpp`, `IMetricsCollector.hpp` в microservice-core
- **Тесты:** Unit-тест: record metrics → Prometheus format output
- **Ссылка:** trading-platform MET-01

### SRV-31: HTTPS/TLS support
- **SP:** 8
- **Модуль:** microservice-boost
- **Что:** Добавить HTTPS сервер: Boost.Asio SSL stream + сертификат/ключ через ServerSettings. Режимы: HTTP only, HTTPS only, HTTP→HTTPS redirect. В trading-platform SEC-04 (TLS in K8s) — но TLS может быть на load balancer, а не на сервере. Однако для standalone deployment HTTPS полезен.
- **Файлы:** `BoostBeastApplication.hpp`/`.cpp`, `ServerSettings.hpp`, новый `SslSettings.hpp`
- **Тесты:** Integration-тест: HTTPS connection → корректный response; self-signed cert → warning

### SRV-32: Async HTTP client
- **SP:** 8
- **Модуль:** microservice-boost
- **Что:** Текущий `HttpClient` синхронный (блокирует поток). Добавить `IAsyncHttpClient` с `sendAsync(request, callback)` на Boost.Asio async. Callback получает response или error. Connection pooling для keep-alive. В trading-platform: circuit breaker (REL-05) и retry (REL-06) требуют async HTTP.
- **Файлы:** Новый `IAsyncHttpClient.hpp`, `AsyncHttpClient.hpp`/`.cpp`
- **Тесты:** Integration-тест: async request → callback; multiple concurrent requests

### SRV-33: WebSocket support
- **SP:** 5
- **Модуль:** microservice-boost
- **Что:** Добавить `IWebSocketHandler` и `WebSocketSession` для real-time уведомлений. В trading-platform FEAT-10 — WebSocket для order status updates. Boost.Beast поддерживает WebSocket из коробки.
- **Файлы:** Новый `IWebSocketHandler.hpp` в core, `WebSocketSession.hpp`/`.cpp` в boost
- **Тесты:** Integration-тест: connect → send message → receive message → close

### ~~SRV-34~~ ✅ ПОГЛОЩЕНО DRY-08 — см. DRY-08 (разделение header-only на hpp/cpp — часть архитектурного порядка)

---

## P3 — Documentation & DX

### SRV-35: API Reference документация
- **SP:** 3
- **Модуль:** docs
- **Что:** Разделить README на отдельные документы: docs/api.md (IRequest/IResponse API), docs/routing.md (RouteMatcher, named params, wildcards), docs/middleware.md (ChainHandler, creating middleware), docs/configuration.md (ServerSettings, config.json), docs/deployment.md (CMake integration, FetchContent, Docker). README оставить как quick start.
- **Файлы:** Новый `docs/api.md`, `docs/routing.md`, `docs/middleware.md`, `docs/configuration.md`, `docs/deployment.md`
- **Ссылка:** CHANGELOG.md v0.3.0 — "Разделение README"

### SRV-36: Миграционный guide с v0.2.0 на v0.3.0
- **SP:** 2
- **Модуль:** docs
- **Что:** При добавлении named params (SRV-11), async server (SRV-28), и других breaking changes — написать docs/migration-v0.3.md с чёткими шагами. Для каждой breaking change: что изменилось, как мигрировать, есть ли backward compatibility.
- **Файлы:** Новый `docs/migration-v0.3.md`
- **Зависит от:** SRV-11, SRV-28

### SRV-37: Versioning и semver для библиотеки
- **SP:** 1
- **Модуль:** CMakeLists.txt
- **Что:** Добавить semver-versioning в CMakeLists.txt: `project(cpp-http-server VERSION 0.3.0)`. Генерировать `version.hpp` с `CPP_HTTP_SERVER_VERSION_MAJOR/MINOR/PATCH`. В `IRequest::getHeader("Server")` добавлять версию. Потребитель может проверить совместимость через CMake version.
- **Файлы:** `CMakeLists.txt`, `version.hpp`
- **Тесты:** Unit-тест: version string format

### ~~SRV-38~~ ✅ ВЫПОЛНЕНО — см. CHANGELOG

### DX-01: Установка C++ плагинов для Cursor IDE
- **SP:** 1
- **Модуль:** DX/инструменты
- **Что:** Установить в Cursor IDE те же C++ плагины, что и в VS Code: `twxs.cmake`, `xaver.clang-format`, `llvm-vs-code-extensions.vscode-clangd`, `vadimcn.vscode-lldb`, `ms-vscode.cpptools`, `ms-vscode.cmake-tools`. Cursor использует свой config-директорий (`~/.config/Cursor/`), поэтому расширения нужно ставить отдельно: `cursor --install-extension <id>`. Также скопировать `settings.json` (format on save, clang-tidy path, CMake presets) из VS Code.
- **Файлы:** Cursor extensions + settings
- **Приоритет:** P3 (низкий)

---

## Сводка по Story Points

| Категория | Задач | SP |
|-----------|-------|-----|
| P1 DRY | 1 | 0 |
| P0 Critical | 1 | 3 |
| P1 Security & Reliability | 3 | 12 |
| P1 API Improvements | 2 | 4 |
| P2 Observability & DX | 2 | 5 |
| P2 Code Quality & Bugs | 2 | 3 |
| P3 Performance & Future | 6 | 43 |
| P3 Documentation & DX | 4 | 7 |
| **Итого** | **23** | **87** |

> Выполненные задачи (в CHANGELOG): DRY-02, DRY-03, DRY-04, DRY-05, DRY-07, DRY-08, SRV-01, SRV-02, SRV-03, SRV-04, SRV-05, SRV-02b, SRV-06, SRV-07, SRV-09, SRV-14, SRV-16, SRV-16a, SRV-16b, SRV-17, SRV-22, SRV-27, SRV-38, SRV-39, SRV-06b