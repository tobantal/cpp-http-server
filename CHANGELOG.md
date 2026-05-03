# Changelog

## [v0.5.0] (2026-05-02)

### Breaking Changes

- **IEnvironment API redesign** (#356) — `get(key, defaultValue)` removed (masked errors); use `get_optional(key).value_or(default)` instead
- `getProperty(key)` now throws `ConvertError(400)` instead of `std::runtime_error` — propagates as HTTP 400 via ChainHandler
- `get<T>(key)` now throws `ConvertError` on missing key or type mismatch (was `std::runtime_error` + `std::bad_any_cast`)

### New Features

- **IEnvironment::get_optional\<T\>(key)** — returns `std::optional<T>`: `nullopt` if key absent, `ConvertError(400)` if type mismatch
- **IEnvironment::hasProperty(key)** — check if key exists without throwing
- **ConvertError(400)** thrown by `getProperty`, `get`, and `get_optional` on type mismatch — automatic HTTP 400 via ChainHandler
- **SchemaValidator** (#360) — request field validation middleware
  - `Schema` base class with fluent DSL: `field<std::string>("name").required().minLength(3)`
  - `ISchemaValidator` port (inherits `IHttpHandler`)
  - `SchemaValidator` adapter: validates `IEnvironment` fields, throws `BadRequestError(400)` with all violations
  - Supports: `required`/`optional`, type checks (string/int/double/bool), `min`/`max`, `minLength`/`maxLength`
  - Place in `ChainHandler` after `JsonProcessor`, before business handler
- **CircuitBreaker** (#359, MSG-03) — circuit breaker pattern for outgoing HTTP calls
  - `ICircuitBreaker` port with states: CLOSED, OPEN, HALF_OPEN
  - `CircuitBreaker` domain model: thread-safe transitions, DNS failures not counted
  - `ICircuitBreakerSettings` + `CircuitBreakerSettings` (env config: `<PREFIX>_CB_FAILURE_THRESHOLD`, `<PREFIX>_CB_RESET_TIMEOUT_MS`, `<PREFIX>_CB_HALF_OPEN_MAX_CALLS`)
  - `CircuitBreakingHttpClient` decorator over `IHttpClient` (returns `ConnectionRefused` when open)
  - 23 unit tests

### Improvements

- **JsonToEnvConverter** — null JSON values are now skipped (not stored as empty `std::any`), so `get_optional<T>("key")` returns `nullopt` for null
- **JsonToEnvConverter / loadJsonToEnvironment** — JSON arrays of strings now produce comma-separated values (`["a","b"]` → `"a,b"`), mixed/number arrays also supported (#140, SRV-24)
- **BoostBeastApplication::loadEnvironment** — config path now resolves via `--config`/`-c` CLI arg > `CONFIG_PATH` env var > `config.json` default (#135, SRV-10)

---

## [v0.4.0](https://github.com/tobantal/cpp-http-server/releases/tag/v0.4.0) (2026-04-30)

### New Features

- **CircuitBreaker** (MSG-03) — CLOSED/OPEN/HALF_OPEN pattern for HttpClient with configurable thresholds
- **RetryPolicy** (MSG-01) — Exponential backoff with configurable retry attempts and delays
- **Repository pattern** (DX-02) — `IRepository` interface with `PostgresKeyValueRepository` and `InMemoryKeyValueRepository` implementations
- **Database layer** (DB-01/DB-02) — `IConnectionPool` / `ConnectionPool` for PostgreSQL connection pooling, `ITransactionExecutor` / `PostgresTransactionExecutor`
- **EventBus** (MSG-01) — `IEventPublisher`, `IEventConsumer`, `DomainEvent`, `InMemoryEventBus` with 22 tests
- **RabbitMQ adapter** (MSG-02) — Publisher/consumer with reconnection, metrics, and lifecycle management
- **HttpLogger / SplunkLogger** (SRV-31) — HTTP-based logging with `IShutdown` graceful flush support
- **UUID v7** (#147) — `Uuid7Generator` according to RFC 9562
- **JSON architecture** (#284) — `IConverter`-based architecture replacing `JsonObject`

### Improvements

- **add_http_service() CMake** (#148) — DRY generation of `main.cpp` files for microservice targets
- **keep-alive support** (SRV-29) — Idle timeout and `maxRequestsPerConnection` configuration
- **Allow header** (SRV-07b) — Added to 405 Method Not Allowed responses
- **BaseWebApplication** (SRV-02c) — Boost-independent logic extracted
- **ServerSettings from ENV** (DRY-04) — Configuration via environment variables

### Bug Fixes

- Fixed connection close: `shutdown_both` instead of `shutdown_send`
- Fixed `Content-Length` header: `res.prepare_payload()` before `http::write`
- Fixed keep-alive idle timeout handling

### Documentation

- Doxygen comments rules added to cpp-style.md
- Architecture documentation (SOLID, GoF, hexagonal architecture)
- README updated for v0.4.0

---

## [v0.3.2](https://github.com/tobantal/cpp-http-server/releases/tag/v0.3.2) (2026-04-27)

### Bug Fixes

- Fixed `Content-Length` header in responses
- Added keep-alive idle timeout (`SERVER_KEEP_ALIVE_TIMEOUT_MS`, default 5s)

---

## [v0.3.1](https://github.com/tobantal/cpp-http-server/releases/tag/v0.3.1) (2026-04-27)

### Bug Fixes

- Fixed connection shutdown: `shutdown_both` instead of `shutdown_send`

---

## [v0.3.0](https://github.com/tobantal/cpp-http-server/releases/tag/v0.3.0) (2026-04-22)

### New Features

- **Graceful Shutdown** — `IShutdown` / `ShutdownManager` with LIFO order and timeout
- **Metrics** (Prometheus) — `IMetricsCollector` / `MetricsCollector`, `MetricsObserverHandler`, `MetricsHandler`
- **Error Handling** — `IHttpErrorHandler` / `HttpErrorSender` with JSON error responses
- **Trace ID** — `X-Trace-ID` propagation through handler chain
- **Handler Names in Logs** — Each handler logs its name and execution time
- **Connection Limits** — DoS protection with 503 responses
- **Request Timeouts** — Read/write timeouts for server and client
- **Request Body Size Limit** — Configurable limit with 413 response
- **CORS Middleware** — `CorsHandler` with `CorsConfig`
- **Health Check** — `HealthHandler` with dependency checks
- **Thread-Safety** — Atomic `running_` flag, mutex-protected handlers map
- **HTTP Exceptions Hierarchy** — `HttpError` base class with derived `NotFoundError`, `BadRequestError`, etc.
- **ILogger abstraction** — `ConsoleLogger`, `NullLogger`, `TestLogger`
- **IResponse extensions** — `HttpStatus` enum, `getReasonPhrase()`, `setCookie()`
- **Environment configuration** — Type-safe config from ENV / config.json / default
- **Trie-based routing** — Efficient route matching with wildcards
- **Named path parameters** — `:param` syntax for URL parameters
- **CI improvements** — Caching, matrix builds, clang-tidy, sanitizers

---

## [v0.2.0](https://github.com/tobantal/cpp-http-server/releases/tag/v0.2.0) (2026-04-20)

### New Features

- **Middleware chain** — `ChainHandler` with `IHttpErrorHandler` injection
- **Routing refactoring** — Improved route matching
- **JSON validation middleware** — Request validation
- **State enum** — Replaced two atomic bools with enum
- **Port configuration** — No longer hardcoded to 80
- **JsonSerializer** — Isolated nlohmann/json from handler code

---

## [v0.1.0](https://github.com/tobantal/cpp-http-server/releases/tag/v0.1.0) (2026-04-18)

### New Features

- **Interface expansion** — Extended `IRequest` / `IResponse` interfaces
- **IIdGenerator** — Thread-safe ID generation with `UuidGenerator` (DIP)
- **Timer** — Execution time measurement utility
- **HealthHandler** — Basic health check endpoint
- **MetricsHandler** — Prometheus metrics endpoint

---

## [v0.0.5](https://github.com/tobantal/cpp-http-server/releases/tag/v0.0.5) (2026-04-15)

### New Features

- Basic HTTP server functionality
- Boost.Beast integration
- Request/Response adapters
- Basic routing
