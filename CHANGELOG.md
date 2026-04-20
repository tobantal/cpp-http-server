# Changelog


Все значимые изменения в проекте документируются в этом файле.


Формат основан на [Keep a Changelog](https://keepachangelog.com/ru/1.0.0/),
проект придерживается [Semantic Versioning](https://semver.org/lang/ru/).


---


## [Unreleased]


### Планируется в v0.3.0

#### Новый функционал
- `MetricsHandler` — эндпоинт для Prometheus метрик

#### Документация
- Разделение README на docs/api.md, docs/routing.md, docs/deployment.md


#### DRY-08: Architectural file structure — error/, handler/, util/ subdirs + forwarding headers
- **microservice-core/include/** restructured into domain subdirectories:
  - `error/` — 14 error classes (HttpError, NotFoundError, BadRequestError, etc.)
  - `handler/` — ChainHandler, HealthHandler
  - `util/` — StringUtils, ThreadSafeMap, PathParamExtractor
  - `settings/` — IEnvironment, Environment (unchanged)
  - root — interfaces (I*), HttpStatus, HttpClientError, RouteMatcher, ConsoleLogger, NullLogger, TestLogger, SimpleRequest, SimpleResponse
- **microservice-boost/include/** restructured:
  - `handler/` — JsonValidator
  - `settings/` — ServerSettings (unchanged)
  - root — BoostBeastApplication, BeastRequestAdapter, BeastResponseAdapter, HttpClient, JsonSerializer
- **20 forwarding headers** created at old paths (`include/NotFoundError.hpp` → `#include "error/NotFoundError.hpp"`) — backward compatible, old `#include` paths still work
- **268/268 tests pass** with zero changes (forwarding headers provide compatibility)
- Architectural rule documented in `.opencode/rules/architecture.md`

#### DRY-07: JsonSerializer — isolate nlohmann/json from handler code
- **JsonParseError** (microservice-core): `HttpError(400)` thrown by `deserialize()` on invalid JSON — handler catches `JsonParseError` via `ChainHandler` → returns 400
- **serialize<T>(const T&)** → `std::string` JSON (requires `to_json` specialization for T)
- **deserialize<T>(const std::string&)** → `T` (requires `from_json` specialization), throws `JsonParseError` on parse/type/out_of_range errors
- Consumer code: `res.setResult(HttpStatus::Ok, "application/json", serialize(dto))` instead of manual `nlohmann::json` assembly
- `nlohmann::json` only referenced in `JsonSerializer.hpp` + `to_json`/`from_json` specializations — handler code is clean
- 10 new tests (3 JsonParseError + 7 JsonSerializer: serialize, deserialize, invalid JSON, missing field, type error, round-trip)
- **Backward compatible:** existing handler code continues to work; `serialize`/`deserialize` are opt-in

#### SRV-04: Connection limit — DoS protection with 503 on limit exceeded
- `IServerSettings::getMaxConnections()` — new virtual method
- `ServerSettings`: `SERVER_MAX_CONNECTIONS` env or `server.maxConnections` config (default: 0 = unlimited)
- `std::atomic<int> activeConnections_` — thread-safe counter in BoostBeastApplication
- When limit reached: accept → send HTTP 503 `{"error": "Service unavailable. Connection limit reached."}` → close socket
- When `maxConnections_ == 0`: unlimited connections (backward compatible default)
- Logging: connection count on accept, 503 warning on limit
- 5 new tests for maxConnections (default, env, config, override, invalid)
- **Backward compatible:** default `maxConnections = 0` means unlimited, same as before

#### SRV-09: HttpClient error handling — HttpClientError, decomposed send, read/write timeouts
- **HttpClientError** enum: None, DnsFailed, ConnectTimeout, ConnectionRefused, WriteTimeout, ReadTimeout, UnknownError
- **HttpClientResult** struct: `error` + `errorMessage` + `ok()` — replaces `bool` return
- **IHttpClient::send()** returns `HttpClientResult` (breaking change: `if (client.send(req, res))` → `if (client.send(req, res).ok())`)
- **IResponse NOT mutated on network error** — only filled on successful HTTP response. Caller distinguishes "server returned 500" vs "connection refused"
- HttpClient decomposed: `connect()` + `sendRequest()` + `readResponse()` private methods
- **Read/write timeouts**: `HTTP_CLIENT_READ_TIMEOUT_MS` (default 30s), `HTTP_CLIENT_WRITE_TIMEOUT_MS` (default 30s)
- `httpClientErrorToString()` utility
- 11 updated/new tests
- **Breaking change**: `IHttpClient::send()` signature changed. Consumer migration: `bool ok = client.send(req, res)` → `auto result = client.send(req, res); if (!result.ok()) { handle(result.error); }`

#### SRV-14: getPath() contract — URL-декодирование
- `StringUtils::urlDecode()`: `%XX` hex-декодирование, `+` → space, invalid `%` passthrough
- `BeastRequestAdapter::getPath()`: URL-декодирование после удаления query string
- `BeastRequestAdapter::getQueryParams()`: URL-декодирование ключей и значений
- `IRequest::getPath()` контракт задокументирован: (1) без query string, (2) URL-декодированный, (3) без trailing slash, (4) начинается с "/"
- 14 новых тестов (8 urlDecode + 6 BeastRequestAdapter URL decoding)
- **Backward compatible:** plain paths и query params без %XX не меняются

#### SRV-16a: ILogger абстракция логирования + ConsoleLogger + NullLogger + TestLogger
- `LogLevel` enum (`: uint8_t`): Debug, Info, Warn, Error
- `ILogger` интерфейс: `log(level, category, message)` — чистый virtual, нулевые зависимости
- `logLevelToString(LogLevel)` — утилита, реализация в LogLevel.cpp
- `ConsoleLogger`: `[LEVEL] [category] message` → stdout (микросервис-core)
- `NullLogger`: no-op (Null Object pattern, default в IWebApplication)
- `TestLogger`: thread-safe log capture с `getEntries()`, `at(index)`, `clear()`, `size()` (микросервис-core)
- `IWebApplication::setLogger()/getLogger()`: инжекция логера, дефолт — NullLogger (backward compatible)
- `ChainHandler::setLogger()`: `std::cerr` заменён на `logger_->log()` (Error level)
- `BoostBeastApplication`: все 38 `std::cerr/cout` заменены на `logger_->log()` с категориями App, Server, Session, HttpClient, Config
- `HttpClient::setLogger()`: `std::cerr/cout` заменены на `logger_->log()`
- 10 новых тестов (LogLevel, NullLogger, TestLogger, ConsoleLogger, ILogger interface)
- **Backward compatible:** без setLogger() поведение как раньше (NullLogger, тихий дефолт; ConsoleLogger для production через setLogger)

#### SRV-17: IResponse расширения — HttpStatus enum, getReasonPhrase(), setCookie()
- **HttpStatus enum** (`HttpStatus.hpp`): `HttpStatus::Ok`, `Created`, `BadRequest`, `NotFound`, `Conflict`, `InternalServerError` и т.д. — заменяют магические числа
- **toInt(HttpStatus)** — конвертация в int
- **getReasonPhrase(int)** / **getReasonPhrase(HttpStatus)** — reason phrase по коду (200 → "OK", 404 → "Not Found" и т.д.)
- **IResponse::setStatus(HttpStatus)** и **IResponse::setResult(HttpStatus, ...)** — перегрузки с enum
- **IResponse::setCookie(name, value, path, httpOnly, secure, maxAge)** — поддержка Set-Cookie с Path, Max-Age, HttpOnly, Secure
- Реализовано в SimpleResponse и BeastResponseAdapter
- 16 новых тестов (HttpStatus enum values, getReasonPhrase, setStatus(HttpStatus), setResult(HttpStatus), setCookie variants)
- **Backward compatible:** `setStatus(int)` и `setResult(int, ...)` по-прежнему работают

#### SRV-18: Tracing middleware — X-Trace-ID в ChainHandler
- `IRequest::getTraceId()` — извлекает `X-Trace-ID` из заголовка или генерирует UUID (ленивый, кешируется в атрибуте `traceId`)
- `IRequest::setTraceId(id)` — ручная установка trace ID (редкий случай)
- `IResponse::setTraceId(id)` — устанавливает `X-Trace-ID` в ответ
- `StringUtils::generateUuid()` — thread-safe UUID v4 (timestamp XOR random + counter XOR random, 32 hex chars, адаптирован из trading-platform)
- **ChainHandler автоматически**: (а) извлекает/генерирует trace ID через `req.getTraceId()`, (б) прокидывает `res.setTraceId()` в конце цепочки и при ошибках, (в) включает `[traceId]` в логи ошибок
- Реализовано в `SimpleRequest`, `SimpleResponse`, `BeastRequestAdapter`, `BeastResponseAdapter`
- Отдельный `TraceHandler` middleware НЕ нужен — ChainHandler выполняет эту функцию
- 8 новых тестов (3 `generateUuid` + 5 `ChainHandlerTraceId`)
- 276/276 tests pass
- **Backward compatible:** `getTraceId()` и `setTraceId()` — новые virtual методы, существующий код не ломается

#### SRV-16b: Logger refactoring — constructor injection, remove setLogger/getLogger
- **IWebApplication**: removed `setLogger()`, `getLogger()`, `logger_` member, `#include "NullLogger.hpp"` from interface — interfaces should not contain implementation details
- **BoostBeastApplication**: `logger_` is now a private member initialized via constructor `explicit BoostBeastApplication(std::shared_ptr<ILogger> logger = std::make_shared<NullLogger>())` — like trading-platform pattern
- **ChainHandler**: removed `setLogger()`, added two constructors — `ChainHandler(handlers...)` (default NullLogger) and `ChainHandler(logger, handlers...)` (explicit injection). Removed all `if (logger_)` null-checks — logger is always valid
- **HttpClient**: removed `setLogger()`, logger injected via constructor `explicit HttpClient(std::shared_ptr<ILogger> logger = std::make_shared<NullLogger>())` — same pattern
- Pattern: `explicit Foo(std::shared_ptr<ILogger> logger = std::make_shared<NullLogger>())` — consistent with trading-platform DI style
- 268/268 tests pass with zero changes to test code (backward compatible — default NullLogger preserves existing behavior)
- **Breaking change**: `IWebApplication::setLogger()` and `IWebApplication::getLogger()` removed, `ChainHandler::setLogger()` removed, `HttpClient::setLogger()` removed. Consumer migration: pass logger via constructor instead of calling setter

#### SRV-38: CI improvements — caching, matrix, linting, sanitizers
- **clang-tidy bug fix:** grep regex `^(error|warning):` never matched clang-tidy output format (`/path/File.cpp:42:5: error:`) — CI was always green regardless of issues. Fixed regex to `:(error|warning):` and added `--warnings-as-errors='*'` flag
- **clang-tidy config (`.clang-tidy`):** scoped `HeaderFilterRegex` to `microservice-(core|boost)/` (was `.*` which included Boost/GTest headers); disabled noisy checks (`cppcoreguidelines-pro-bounds-*`, `cppcoreguidelines-avoid-magic-numbers`, `cppcoreguidelines-avoid-c-arrays`, `cppcoreguidelines-pro-type-reinterpret-cast`, `cppcoreguidelines-pro-type-union-access`, `readability-magic-numbers`, `readability-identifier-length`, `readability-function-cognitive-complexity`, `bugprone-easily-swappable-parameters`, `modernize-use-trailing-return-type`); added `concurrency-*` checks (critical for multithreaded code); set `WarningsAsErrors: '*'`; added `FormatStyle: file`
- **ccache integration:** `CMAKE_CXX_COMPILER_LAUNCHER=ccache` in all jobs (build, sanitize, lint) — 50-80% build time reduction on cache hits
- **Cache improvements:** removed `build/CMakeFiles` from cache (stale-state risk, zero savings); separate cache keys for deps vs ccache; added `github.run_id` to ccache key for invalidation on dependency changes
- **Matrix additions:**
  - GCC Release + hardening flags (`-D_FORTIFY_SOURCE=2 -fstack-protector-strong -D_GLIBCXX_ASSERTIONS`)
  - ThreadSanitizer (TSan) in sanitize matrix — critical for project with `std::atomic`, `std::mutex`, `std::thread`
  - Total matrix: 4 build configs (gcc/Debug+coverage, gcc/Release+hardening, clang/Release, macOS/clang/Release) + 3 sanitizers (ASan, UBSan, TSan) + 1 lint job
- **`.clang-format`:** added project code style config (Allman braces, 4-space indent, 120-column limit, left-aligned pointers/references, C++17)
- **268/268 tests pass** with zero changes to production code
- **Backward compatible:** no production code changes, CI-only

#### DRY-03: Тощая поставка библиотеки — find_package + FetchContent fallback
- Корневой CMakeLists.txt реорганизован: `find_package` приоритет + `FetchContent` fallback (опция `CPP_HTTP_SERVER_FETCH_DEPS`, default ON)
- При standalone-сборке (CPP_HTTP_SERVER_FETCH_DEPS=ON) зависимости подтягиваются FetchContent, как раньше
- Consumer-проекты, уже имеющие Boost/nlohmann_json через свой FetchContent или system install, устанавливают `CPP_HTTP_SERVER_FETCH_DEPS=OFF` — без двойного скачивания
- **Boost.DI удалён** — не используется ни в одном файле проекта (0 include'ов)
- GoogleTest: дублирующий FetchContent убран из test/CMakeLists.txt — единый FetchContent в корневом CMake
- Версия проекта: `project(cpp-http-server VERSION 0.3.0)` вместо `MicroservicesProject VERSION 1.0.0`
- microservice-core по-прежнему не имеет внешних зависимостей (header-only + RouteMatcher.cpp)

#### SRV-27: JSON injection vulnerability — ChainHandler::sendError экранирование
- `StringUtils::escapeJson()` — новый метод для экранирования спецсимволов JSON: `"`, `\`, `\n`, `\r`, `\t`, control characters (< 0x20 → `\uXXXX`)
- `ChainHandler::sendError()` использует `escapeJson()` для message — закрывает уязвимость JSON injection
- `BoostBeastApplication::handleRequest()` использует `escapeJson()` для `e.message()` в HttpError catch
- 10 новых тестов StringUtilsTest (escapeJson: plain, empty, quotes, backslash, newline, CR, tab, control chars, multiple, injection attack)
- 2 новых теста ChainHandlerTest (HttpError с кавычками, std::exception не раскрывает сообщение)
- **Backward compatible:** обычные сообщения без спецсимволов не меняются
- **Backward compatible:** при standalone-сборке поведение не меняется (CPP_HTTP_SERVER_FETCH_DEPS=ON по умолчанию)

### Выполнено в v0.3.0 (feature/v0.3.0)

#### SRV-06b: Connect timeout — HttpClient
- `HttpClient::send()` больше не висит при недоступном сервере — `async_connect` с `steady_timer` таймаутом
- Конструктор `HttpClient()` читает `HTTP_CLIENT_CONNECT_TIMEOUT_MS` из ENV (default: 5000ms)
- При таймауте: `stream.close()` + return false + response status 500
- При ошибке connect (connection refused, DNS failure): return false + response status 500
- Точечные логи: `[HttpClient] Connect error: timeout` / `[HttpClient] Connect error: <message>`
- 4 новых теста: ConnectTimeoutOnUnreachableHost (~500ms timeout), ConnectTimeoutRespectsEnvVariable (~300ms), InvalidHostReturnsError (DNS fail), DefaultConnectTimeoutIs5000 (connection refused)
- **Backward compatible:** интерфейс `IHttpClient::send()` не изменён, поведение для успешных запросов не затронуто

#### SRV-22: Port hardcoded to 80 в BeastRequestAdapter
- `BeastRequestAdapter::getPort()` возвращал хардкод 80 — теперь принимает port в конструкторе (default=80 для backward compatibility)
- `BoostBeastApplication::handleSession()` извлекает реальный порт из `socket.local_endpoint().port()` и передаёт в `handleBeastRequest()`
- 3 новых теста: GetPortDefault(80), GetPortExplicit(8080), GetPortCustom(443)

#### SRV-39: JSON request validation middleware
- `JsonValidator` — middleware (IHttpHandler) для проверки валидности JSON в теле запроса
- Проверяет `Content-Type: application/json` через `IRequest::isJson()`
- Парсит тело через `nlohmann::json::parse()` — невалидный JSON → `BadRequestError(400)`
- Интегрируется в `ChainHandler` как первый обработчик перед бизнес-логикой
- 15 новых тестов JsonValidatorTest: валидный/невалидный JSON, пустое тело, Content-Type, цепочка с обработчиком
- **Backward compatible:** новый класс, не ломает существующий код

#### SRV-02b: State enum вместо двух atomic bool
- `enum class ServerState { NotStarted, Running, Stopped }` заменяет `std::atomic<bool> running_` + `std::atomic<bool> started_`
- `state_` — единая `std::atomic<ServerState>` в BoostBeastApplication
- `registerHandler()` разрешён только в `NotStarted` (иначе `std::logic_error`)
- `stop()` использует `compare_exchange_strong(Running, Stopped)` — атомарный переход, повторный вызов — noop
- `start()` устанавливает `Running`, accept loop проверяет `state_ == Running`
- Невалидные комбинации состояний невозможны на уровне типа
- Добавляет возможность расширения: `Starting`, `Draining` в будущем
- 6 новых тестов ServerStateTest
- **Backward compatible:** поведение не изменилось

#### SRV-06: Request timeout (read/write) — сервер
- `beast::tcp_stream` заменяет `tcp::socket` в `handleSession`
- Read timeout: `SERVER_READ_TIMEOUT_MS` (default: 30000) — `stream.expires_after()` перед `http::read()`
- Write timeout: `SERVER_WRITE_TIMEOUT_MS` (default: 30000) — `stream.expires_after()` перед `http::write()`
- I/O timeout логируется как `[Session] Timeout` и соединение закрывается (без HTTP-ответа — клиент не слушает)
- `IServerSettings::getReadTimeout()` / `getWriteTimeout()` — новые методы (возвращают `std::chrono::milliseconds`)
- `RequestTimeoutError` (408) и `GatewayTimeoutError` (504) — добавлены для consumer-кода (низкий приоритет)
- 7 новых тестов ServerSettings (timeout defaults, ENV, config)

#### SRV-07: HTTP method not allowed — 405 MethodNotAllowedError
- `MethodNotAllowedError` (405) — новый класс исключения
- `BoostBeastApplication::handleRequest()` проверяет `pathExists()` — если маршрут найден, но метод не совпадает → `throw MethodNotAllowedError`
- `pathExists()` — новый метод для проверки существования маршрута без учёта метода
- 2 новых теста MethodNotAllowedError
- **Note:** Заголовок `Allow` не добавлен — TODO на будущее

#### SRV-15: Duplicate code — DRY рефакторинг
- Новый `StringUtils.hpp` (microservice-core): `toLower()`, `splitPath()` — единая реализация вместо 4 копий
- Новый `PathParamExtractor.hpp` (microservice-core): `getByIndex()` — единая логика извлечения path params
- SimpleRequest, SimpleResponse, BeastRequestAdapter, BeastResponseAdapter — удалены дублирующиеся `toLower()`, `splitPath()`, getPathParam логика; используются StringUtils и PathParamExtractor
- 16 новых тестов (StringUtils: 11, PathParamExtractor: 5)

#### SRV-05: Request body size limit
- `IServerSettings::getMaxRequestBodySize()` — новый метод интерфейса
- `ServerSettings` читает `SERVER_MAX_REQUEST_BODY_SIZE` из ENV, приоритет: ENV → config.json → дефолт (1MB)
- `BoostBeastApplication` ограничивает `flat_buffer` и проверяет `req.body().size()` → 413 Payload Too Large
- 5 новых тестов ServerSettings

#### SRV-03: Утечка lifetime — detached threads заменены на joinable threads
- `std::thread(...).detach()` → `std::vector<std::thread>` с `std::mutex`
- `stop()` закрывает acceptor, затем join'ит все потоки — нет use-after-free
- `threads_` защищён `threadsMutex_` для thread-safety

#### SRV-02: Thread-safety — handlers_ map защита от регистрации после start()
- Добавлен `started_` флаг (`std::atomic<bool>`) в BoostBeastApplication
- `registerHandler()` бросает `std::logic_error` если вызван после `start()`
- Документировано: модификация handlers_ только до start()

#### SRV-01: Thread-safety — running_ flag atomic
- `running_` → `std::atomic<bool>` в BoostBeastApplication
- `stop()` использует `exchange(false)` — атомарная проверка+сброс
- `start()` использует `store(true)` / `load()` в цикле
- Signal handler и main thread больше не имеют data race

#### DRY-02: Иерархия HTTP-исключений + ChainHandler try-catch модель
- Новый базовый класс `HttpError(statusCode, message)` с геттерами
- 9 классов исключений (1 класс = 1 файл): `BadRequestError` (400), `UnauthorizedError` (401), `ForbiddenError` (403), `NotFoundError` (404), `ConflictError` (409), `InternalError` (500), `ServiceUnavailableError` (503), `BusinessError` (400), `AuthError` (401)
- `ChainHandler` переработан: try-catch модель вместо проверки `status==0`
  - `HttpError` → JSON ответ с соответствующим статусом, цепочка прерывается
  - `std::exception` → 500 Internal Server Error, цепочка прерывается
  - Handler без исключения → цепочка продолжается, ответ не трогается
  - Пустая цепочка без исключения → status остаётся 200 (дефолт)
- `BoostBeastApplication::handleRequest` ловит `HttpError` отдельно от `std::exception`
- **Backward compatible:** старый код с `res.setResult()` продолжает работать
- 18 тестов HttpError + 8 тестов ChainHandler

#### DRY-04: ServerSettings — хост/порт из ENV переменных
- `ServerSettings` читает `SERVER_HOST` и `SERVER_PORT` из ENV переменных
- Приоритет: ENV → config.json → дефолт (0.0.0.0:8080)
- Упрощает деплой в K8s: не нужен config.json для хоста/порта
- Backward compatible: config.json продолжает работать
- 7 новых тестов покрывают все комбинации

#### DRY-05: Выпилить IDbSettings / DbSettings
- Удалены: `IDbSettings.hpp`, `DbSettings.hpp`, `DbSettings.cpp`, `DbSettingsTest.cpp`
- Работа с БД — не удел библиотеки HTTP-сервера, микросервисы сами управляют настройками БД
- IEnvironment остаётся как универсальный механизм конфигурации
- **Breaking change:** consumer-проекты, использующие `DbSettings` — нужно перенести в свой код. В trading-platform уже есть свой `common::settings::DbSettings`


#### Документация
- Разделение README на docs/api.md, docs/routing.md, docs/deployment.md


### Планируется в будущих версиях


#### Технический долг
- Кодогенерация DI из di.json
- Разделение hpp/cpp файлов
- Named path parameters (`:orderId` синтаксис)
- Trie-based routing для эффективного матчинга


---


## [0.2.0] - 2026-01-22


### Middleware и рефакторинг роутинга


#### Новые компоненты
- `ChainHandler` — middleware цепочка обработчиков
- `HealthHandler` — базовый health-check эндпоинт


#### Изменения в IWebApplication
- `registerEndpoint(method, pattern, handlers...)` — публичный API для регистрации middleware цепочки
- `registerHandler()` — перенесён в protected (внутренний механизм)


#### Изменения в BoostBeastApplication
- **Breaking:** `handlers_` — приватная переменная
- **Breaking:** Новая структура: `map<pattern, map<method, handler>>`
- `HandlerMatch` — приватная вложенная структура
- Удалён `HANDLER_KEY_DELIMITER`
- Удалён `getHandlerKey()`


#### Миграция с v0.1.0
```cpp
// Было (v0.1.0):
handlers_[getHandlerKey("GET", "/api/orders")] = ordersHandler;


// Стало (v0.2.0) — middleware цепочка:
registerEndpoint("GET", "/api/orders",
    authMiddleware,
    loggingMiddleware,
    ordersHandler);


// Стало (v0.2.0) — один handler:
registerEndpoint("GET", "/health",
    std::make_shared<HealthHandler>());
```


---


## [0.1.0] - 2026-01-15


### Расширение интерфейсов


#### IRequest
- `getQueryParams()`, `getQueryParam(name)` — работа с query параметрами
- `getHeader(name)` — case-insensitive получение заголовка
- `getBearerToken()` — извлечение Bearer токена
- `getPathPattern()`, `setPathPattern()`, `getPathParam(index)` — path parameters
- `getPathSegments()` — разбиение пути на сегменты
- `setAttribute()`, `getAttribute()` — передача данных между middleware
- `isJson()`, `getContentType()` — работа с Content-Type
- `setBody()`, `setHeader()`, `setHeaders()` — сеттеры


#### IResponse
- `getStatus()`, `getBody()`, `getHeaders()`, `getHeader(name)` — геттеры
- `setResult(code, contentType, body)` — convenience метод


#### BoostBeastApplication
- `HandlerMatch` — возврат handler + pattern из findHandler()
- Поддержка path parameters через `setPathPattern()`


---


## [0.0.5] - 2025-12-15


### Базовый функционал


#### Core Module
- `IRequest`, `IResponse` — интерфейсы запроса/ответа
- `IWebApplication` — Template Method паттерн
- `IHttpHandler`, `IHttpClient` — интерфейсы обработчика и клиента
- `IEnvironment`, `Environment` — конфигурация
- `RouteMatcher` — wildcard маршрутизация
- `SimpleRequest`, `SimpleResponse` — реализации для тестов


#### Boost Module
- `BoostBeastApplication` — HTTP-сервер на Boost.Beast/Asio
- `BeastRequestAdapter`, `BeastResponseAdapter` — адаптеры
- `HttpClient` — синхронный HTTP-клиент
- `ServerSettings` — настройки


---


[Unreleased]: https://github.com/tobantal/cpp-http-server/compare/v0.3.0...HEAD
[0.3.0]: https://github.com/tobantal/cpp-http-server/releases/tag/v0.3.0
[0.2.0]: https://github.com/tobantal/cpp-http-server/releases/tag/v0.2.0
[0.1.0]: https://github.com/tobantal/cpp-http-server/releases/tag/v0.1.0
[0.0.5]: https://github.com/tobantal/cpp-http-server/releases/tag/v0.0.5
