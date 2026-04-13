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

### DRY-02: Иерархия HTTP-исключений + HttpErrorHandler ✅ ВЫПОЛНЕНО
- **SP:** 5
- **Модуль:** microservice-core, microservice-boost
- **Что:** Создать стандартную иерархию исключений и переработать ChainHandler на try-catch модель. Handler бросает исключение (`throw NotFoundError("Order not found")`), ChainHandler ловит его по типу и формирует HTTP-ответ. Каждое исключение несёт человекочитаемое имя + HTTP-код (приватное поле `statusCode_`). Иерархия: HttpError (base) → BadRequestError(400), UnauthorizedError(401), ForbiddenError(403), NotFoundError(404), ConflictError(409), InternalError(500), ServiceUnavailableError(503), BusinessError(400), AuthError(401). 1 класс = 1 файл. ChainHandler: HttpError → error response + stop, std::exception → 500 + stop, no exception → continue. Формат ошибки: `{"error": "<message>"}`.
- **Файлы:** `HttpError.hpp`, `BadRequestError.hpp`, `UnauthorizedError.hpp`, `ForbiddenError.hpp`, `NotFoundError.hpp`, `ConflictError.hpp`, `InternalError.hpp`, `ServiceUnavailableError.hpp`, `BusinessError.hpp`, `AuthError.hpp` (microservice-core/include); `ChainHandler.hpp` (переработан); `BoostBeastApplication.cpp` (HttpError catch); `HttpErrorTest.cpp`, `ChainHandlerTest.cpp` (новые)
- **Тесты:** 18 тестов HttpError (каждый класс, catch по ссылке, catch std::exception), 8 тестов ChainHandler (нормальное завершение, 201 статус, NotFoundError, UnauthorizedError, std::exception, middleware прерывает цепочку, пустая цепочка)
- **Ссылка:** trading-platform REF-09 (httpStatus workaround — теперь не нужен), REF-13 (exception hierarchy)

### DRY-03: Аналитика — разделение «тощего» кода библиотеки и внешних зависимостей
- **SP:** 3
- **Модуль:** CMake (корневой)
- **Что:** Сейчас библиотека «жирная» — FetchContent в корневом CMakeLists.txt тянет: Boost 1.83.0 (весь tarball ~33MB, хотя нужны только system/beast/asio), Boost.DI v1.3.0, nlohmann/json v3.11.3. При подключении через FetchContent consumer-проект получает чужие транзитивные зависимости с захардкоженными версиями — риск конфликта. Проанализировать варианты поставки библиотеки, где в репозитории — только наш код, а зависимости подтягиваются consumer-проектом. Варианты: (а) `find_package()` — потребитель сам предоставляет зависимости через систему (apt, vcpkg, conan, свой FetchContent); (б) FetchContent как опциональный fallback (`CPP_HTTP_SERVER_FETCH_DEPS=ON`); (в) CMake PackageConfig (`cpp-http-serverConfig.cmake`) — install + find_package для consumer; (г) разделить microservice-core (нулевой зависимости) и microservice-boost (зависимости Consumer обеспечивает). microservice-core вообще не должен требовать FetchContent — у него нет сторонних зависимостей. Результат — документ с рекомендацией по варианту + задача на реализацию.
- **Файлы:** `CMakeLists.txt` (корневой), `microservice-core/CMakeLists.txt`, `microservice-boost/CMakeLists.txt`, `README.md` (раздел Installation)
- **Критерий успеха:** Consumer может подключить cpp-http-server через FetchContent без двойного скачивания Boost; microservice-core подключается вообще без зависимостей
- **Результат:** Документ с анализом + рекомендация по варианту (пока документируем, реализация — отдельная задача)

### DRY-04: Хост/порт из ENV переменных (ServerSettings) ✅ ВЫПОЛНЕНО
- **SP:** 3
- **Модуль:** microservice-boost
- **Что:** `ServerSettings` читает хост и порт из ENV: `SERVER_HOST` (default: "0.0.0.0"), `SERVER_PORT` (default: 8080). Приоритет: ENV → config.json → дефолт.
- **Файлы:** `ServerSettings.hpp`/`.cpp`, `IServerSettings.hpp`
- **Тесты:** 7 тестов: ENV only, ENV overrides config, config only, defaults, mixed, invalid port ENV, invalid port defaults
- **Совместимость:** Backward compatible — config.json продолжает работать

### DRY-04b: Аналитика — выпиливание nlohmann/json и config.json
- **SP:** 3
- **Модуль:** microservice-boost, CMake
- **Что:** Проанализировать возможность удалить nlohmann/json и config.json из библиотеки. Сейчас `loadEnvironment()` читает `config.json` через nlohmann/json и вызывает `loadJsonToEnvironment()`. Альтернатива: consumer сам наполняет IEnvironment, а библиотека не парсит конфиг вообще. Но config.json может хранить настройки, которые неудобно передавать через ENV (имя сервера, настройки логирования в будущем). Возможный компромисс: оставить nlohmann/json + config.json для backward compatibility, но парсить значения в ENV через `setenv()` — тогда consumer может читать их через `std::getenv()` единообразно. Результат — документ с рекомендацией.
- **Файлы:** Анализ: `BoostBeastApplication.cpp` (loadJsonToEnvironment), `CMakeLists.txt`
- **Результат:** Документ с анализом + рекомендация (пока документируем, реализация — отдельная задача)
- **Связанные задачи:** DRY-03 (тощая поставка — после выпиливания json задача упрощается)

### DRY-05: Выпилить IDbSettings / DbSettings из библиотеки ✅ ВЫПОЛНЕНО
- **SP:** 1
- **Модуль:** microservice-core, microservice-boost
- **Что:** Удалить `IDbSettings.hpp`, `DbSettings.hpp`/`.cpp` из cpp-http-server. Работа с БД — не удел библиотеки HTTP-сервера. Микросервисы сами отвечают за создание настроек подключения к БД и их наполнение (как уже сделано в trading-platform — свой `common::settings::DbSettings` с ENV). Оставить IEnvironment как универсальный механизм конфигурации — потребитель может хранить любые настройки, включая DB.
- **Файлы:** Удалено: `microservice-core/include/settings/IDbSettings.hpp`, `microservice-boost/include/settings/DbSettings.hpp`, `microservice-boost/src/settings/DbSettings.cpp`, `microservice-boost/tests/DbSettingsTest.cpp`; обновлено: `CMakeLists.txt`
- **Тесты:** DbSettingsTest удалён; остальные тесты прошли без изменений

### DRY-06: Аналитика — оценка библиотек для возможного внедрения
- **SP:** 2
- **Модуль:** microservice-core, microservice-boost
- **Что:** Проанализировать библиотеки из LIBRARIES.md trading-platform на применимость к cpp-http-server. Фокус на тех, что напрямую улучшают библиотеку сервера. Кандидаты для анализа: (а) **absl::StatusOr<T>** — замена `bool` return в IHttpClient::send(), замена `std::optional` в error paths, более информативный паттерн ошибок; (б) **absl::flat_hash_map** — замена `std::map` для handlers_ (быстрее lookup для роутинга); (в) **boost::lockfree::queue** — для thread-safe очереди задач при переходе на thread pool (SRV-03/SRV-04); (г) **string_view** (C++17, уже доступен) — замена `const std::string&` в IRequest/IResponse методах, устранение лишних аллокаций. Отдельный вопрос: стоит ли добавлять зависимость от Abseil ради StatusOr/flat_hash_map, или проще реализовать аналог своими силами. Результат — документ с рекомендацией по каждой библиотеке.
- **Результат:** Документ с анализом + рекомендации (пока документируем, реализация — отдельная задача)

---

## P0 — Critical (блокирует production)

### SRV-01: Thread-safety — `running_` flag не atomic ✅ ВЫПОЛНЕНО
- **SP:** 1
- **Модуль:** microservice-boost
- **Что:** `running_` → `std::atomic<bool>`, `stop()` использует `exchange(false)`, `start()` использует `store(true)` / `load()`. Data race устранён.

### SRV-02: Thread-safety — `handlers_` map без защиты
- **SP:** 2
- **Модуль:** microservice-boost
- **Что:** `handlers_` (map<string, map<string, shared_ptr<IHttpHandler>>>) — доступ из detached threads без синхронизации. Сейчас безопасно, т.к. регистрация происходит до start(), но нет формальной гарантии. Варианты: (1) документировать что модификация только до start(), (2) использовать ThreadSafeMap, (3) immutable map после init. Рекомендация: вариант (1) + assert в registerHandler что start() ещё не вызван.
- **Файлы:** `BoostBeastApplication.hpp`, `BoostBeastApplication.cpp`
- **Тесты:** Unit-тест: registerHandler после start() → assert/exception

### SRV-03: Утечка lifetime — detached threads после stop()
- **SP:** 5
- **Модуль:** microservice-boost
- **Что:** `handleSession` запускается в `std::thread(...).detach()`. При вызове `stop()` (деструктор) detached threads продолжают работать, но `this` уже уничтожен → use-after-free. Решение: заменить detached threads на thread pool + join при stop(). Добавить graceful shutdown: `io_context.stop()` + `thread_pool.join()` с таймаутом.
- **Файлы:** `BoostBeastApplication.hpp`, `BoostBeastApplication.cpp`
- **Тесты:** Integration-тест: start → send request → stop → no crash; start → stop → restart

### SRV-04: Неограниченное создание потоков — DoS-уязвимость
- **SP:** 5
- **Модуль:** microservice-boost
- **Что:** Каждый коннект создаёт новый detached thread без лимита. При flash-crowd или DoS — исчерпание ресурсов. Решение: thread pool с фиксированным размером (configurable, default = std::thread::hardware_concurrency()). Если пул полон — отклонять новые соединения (503 Service Unavailable) или ставить в очередь.
- **Файлы:** `BoostBeastApplication.hpp`, `BoostBeastApplication.cpp`, новый `ThreadPool.hpp`
- **Тесты:** Integration-тест: pool exhaustion → 503; pool recovery → новый запрос обслуживается
- **Зависит от:** SRV-03 (сначала thread pool)

---

## P1 — Security & Reliability

### SRV-05: Request body size limit
- **SP:** 3
- **Модуль:** microservice-boost
- **Что:** Нет лимита на размер request body. Клиент может отправить multi-GB body → OOM. Добавить `maxRequestBodySize` в ServerSettings (default: 1MB). При превышении → 413 Payload Too Large.
- **Файлы:** `BoostBeastApplication.cpp`, `ServerSettings.hpp`, `IServerSettings.hpp`
- **Тесты:** Integration-тест: отправка body > limit → 413

### SRV-06: Request timeout
- **SP:** 3
- **Модуль:** microservice-boost
- **Что:** `http::read()` в handleSession не имеет таймаута. Медленный клиент может блокировать поток бесконечно. Добавить `beast::tcp_stream::expires_after()` для connect/read/write. Настройки в ServerSettings: `readTimeoutMs` (default: 30000), `writeTimeoutMs` (default: 30000).
- **Файлы:** `BoostBeastApplication.cpp`, `ServerSettings.hpp`, `IServerSettings.hpp`
- **Тесты:** Integration-тест: медленный клиент → timeout → соединение закрыто

### SRV-07: HTTP method not allowed — 405 ответ
- **SP:** 2
- **Модуль:** microservice-boost
- **Что:** Если маршрут найден, но метод не совпадает (например, POST на GET-only маршрут), сервер возвращает 404. Это некорректно по HTTP spec. Вернуть 405 Method Not Allowed + заголовок `Allow: GET`.
- **Файлы:** `BoostBeastApplication.cpp` (в findHandler или handleRequest)
- **Тесты:** Integration-тест: POST на GET-only маршрут → 405 + Allow header

### SRV-08: Graceful shutdown
- **SP:** 3
- **Модуль:** microservice-boost
- **Что:** При SIGTERM/SIGINT: прекратить приём новых соединений, доработать текущие запросы (с таймаутом 5s), закрыть все соединения, остановить io_context. Добавить `gracefulShutdown()` метод и signal handler в `run()`.
- **Файлы:** `BoostBeastApplication.hpp`, `BoostBeastApplication.cpp`
- **Тесты:** Integration-тест: SIGTERM → graceful shutdown → текущий запрос завершается

### SRV-09: HttpClient timeouts и error handling
- **SP:** 5
- **Модуль:** microservice-boost
- **Что:** HttpClient::send() синхронный, без таймаутов, создаёт новое соединение на каждый запрос. Добавить: (1) connect timeout, read timeout, write timeout (default: 5s каждый, configurable через HttpClientSettings), (2) корректный error code enum вместо bool, (3) connection pooling или хотя бы keep-alive. В trading-platform это критично для HttpAuthClient и HttpBrokerGateway.
- **Файлы:** `HttpClient.hpp`, `HttpClient.cpp`, новый `IHttpClient.hpp` (расширить), `HttpClientSettings.hpp`
- **Тесты:** Unit-тест: timeout → error code; integration-тест с mock-сервером
- **Ссылка:** trading-platform REL-05 (Circuit Breaker), REL-06 (Retry), BUG-07a (silent exception swallowing)

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

### SRV-12: HTTP status workaround cleanup — REF-09
- **SP:** 3
- **Модуль:** microservice-core
- **Что:** В trading-platform `CreateOrderHandler` использует `req.setAttribute("httpStatus", "201")` хак для передачи статус-кода через цепочку middleware. Корень проблемы: ChainHandler использует status=0 для «продолжить цепочку», но handler'у нужно вернуть 201/200. Решение: (1) добавить `IResponse::setDeferredStatus(code)` — отложенный статус, который ChainHandler применит в конце цепочки, или (2) изменить семантику: middleware не должна менять status 0, а handler может вернуть любой статус — ChainHandler вызывает post-middleware в любом случае.
- **Файлы:** `IResponse.hpp`, `ChainHandler.hpp`, `BoostBeastApplication.cpp`
- **Тесты:** Unit-тест: handler возвращает 201 + post-middleware выполняется; middleware возвращает 401 + chain останавливается
- **Ссылка:** trading-platform REF-09

### SRV-13: Trie-based routing — FUT-01
- **SP:** 8
- **Модуль:** microservice-core
- **Что:** Заменить O(n) линейный scanning по wildcard-паттернам на O(k) trie lookup, где k = длина URL. Поддержка exact match, named parameters (`:id`), wildcards (`*`). Радикальное ускорение для 33+ endpoint-ов (в trading-platform), и масштабируемость для 100+.
- **Файлы:** Новый `TrieRouter.hpp`/`.cpp` в microservice-core, `BoostBeastApplication.cpp` (замена findHandler)
- **Тесты:** Benchmark: trie vs linear для 33, 100, 500 маршрутов; Unit-тесты: exact, named, wildcard, priority (most specific match)
- **Зависит от:** SRV-11 (named params — сначала определить API)

### SRV-14: `getPath()` contract — REF-10
- **SP:** 1
- **Модуль:** microservice-core
- **Что:** Задокументировать контракт `getPath()`: путь без query string, без trailing slash, URL-decoded или нет. Сейчас `BeastRequestAdapter::getPath()` убирает query string, но не декодирует URL. Добавить URL-декодирование path segments. В trading-platform: `QuotesHandler` обрабатывает `figis=...` query param — нужно гарантировать корректный парсинг.
- **Файлы:** `IRequest.hpp` (документация), `BeastRequestAdapter.hpp`/`.cpp` (URL decode)
- **Тесты:** Unit-тест: `getPath()` с encoded URL → decoded result

### SRV-15: Duplicate code — DRY рефакторинг
- **SP:** 2
- **Модуль:** microservice-core + microservice-boost
- **Что:** `toLower()` дублируется в 4 файлах (SimpleRequest, SimpleResponse, BeastRequestAdapter, BeastResponseAdapter). `splitPath()` дублируется в SimpleRequest и BeastRequestAdapter. `getPathParam()` логика дублируется. Вынести в общий utility: `StringUtils.hpp` (toLower, splitPath) и `PathParamExtractor.hpp` (getPathParam логика) в microservice-core.
- **Файлы:** Новый `microservice-core/include/StringUtils.hpp`, `PathParamExtractor.hpp`; правки в SimpleRequest, SimpleResponse, BeastRequestAdapter, BeastResponseAdapter
- **Тесты:** Unit-тест для StringUtils и PathParamExtractor; запустить все существующие тесты

---

## P2 — Observability & Developer Experience

### SRV-16: Structured logging (ILogger integration)
- **SP:** 3
- **Модуль:** microservice-boost
- **Что:** Заменить `std::cout/std::cerr` в BoostBeastApplication и HttpClient на ILogger interface (инжектируемый через DI или setter). Логировать: request start (method, path, IP), request end (status, duration), errors, connections. В trading-platform уже есть ILogger с Quill — нужно сделать библиотеку совместимой.
- **Файлы:** `BoostBeastApplication.hpp`/`.cpp`, `HttpClient.hpp`/`.cpp`, новый `IServerLogger.hpp` или integration point
- **Тесты:** Unit-тест: request → log output содержит expected fields
- **Ссылка:** trading-platform OBS-01 (ILogger + Quill)

### SRV-17: `IResponse` расширения
- **SP:** 3
- **Модуль:** microservice-core
- **Что:** Добавить в IResponse: (1) `getStatusCode()` — получить reason phrase по коду (например, 201 → "Created"), (2) `setCookie(name, value, options)` — поддержка Set-Cookie заголовка, (3) HTTP status constants — `HttpStatus::OK`, `HttpStatus::CREATED`, `HttpStatus::NOT_FOUND`, `HttpStatus::METHOD_NOT_ALLOWED`. В trading-platform статусы захардкодены как числа.
- **Файлы:** `IResponse.hpp`, `SimpleResponse.hpp`, `BeastResponseAdapter.hpp`, новый `HttpStatus.hpp`
- **Тесты:** Unit-тест для каждого нового метода

### SRV-18: Tracing middleware support (X-Trace-ID)
- **SP:** 2
- **Модуль:** microservice-core
- **Что:** Добавить встроенную поддержку X-Trace-ID в библиотеку: (1) `IRequest::getTraceId()` — извлечь X-Trace-ID или сгенерировать UUID, (2) `IResponse::setTraceId()` — пробросить в ответ. В trading-platform OBS-02 (TracingMiddleware) это реализовано вручную.
- **Файлы:** `IRequest.hpp`, `IResponse.hpp`, `SimpleRequest.hpp`, `SimpleResponse.hpp`, `BeastRequestAdapter.hpp`, `BeastResponseAdapter.hpp`
- **Тесты:** Unit-тест: request с X-Trace-ID → response с тем же ID; request без → response с новым UUID
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
- **Если:** Browser-based API клиенты требуют CORS headers. Добавить встроенный `CorsHandler` middleware: OPTIONS preflight → 204 + CORS headers, остальные методы → CORS headers в ответе. Настройки: `allowedOrigins`, `allowedMethods`, `allowedHeaders`.
- **Файлы:** Новый `CorsHandler.hpp` в microservice-core
- **Тесты:** Unit-тест: OPTIONS preflight → 204 + CORS; GET с Origin → CORS headers

---

## P2 — Code Quality & Bug Fixes

### SRV-21: ChainHandler short-circuit semantics — документация и edge cases
- **SP:** 2
- **Модуль:** microservice-core
- **Что:** Статус 0 = «продолжить», статус != 0 = «остановить» — неочевидный контракт. Документировать в ChainHandler.hpp. Обработать edge case: handler не вызвал setStatus() → status 0 → chain продолжает → в конце 500. Добавить вариант: `IHttpHandler::handle()` возвращает `bool` или `ChainResult` (Continue/Handled/Error). Рассмотреть как опцию для v0.3.0.
- **Файлы:** `ChainHandler.hpp`, `IHttpHandler.hpp`
- **Тесты:** Unit-тест: empty chain → 500; single handler → correct status; last handler sets 0 → 500

### SRV-22: Port hardcoded to 80 в BeastRequestAdapter
- **SP:** 1
- **Модуль:** microservice-boost
- **Что:** `BeastRequestAdapter::getPort()` возвращает хардкод 80. Нужно извлекать реальный порт из `req_` (через endpoint) или передавать в конструктор.
- **Файлы:** `BeastRequestAdapter.hpp`
- **Тесты:** Unit-тест: port != 80 → возвращается корректный порт

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

### SRV-25: `IHttpClient::send()` возвращает bool — недостаточно информативно
- **SP:** 3
- **Модуль:** microservice-core + microservice-boost
- **Что:** `IHttpClient::send()` возвращает `bool` (success/failure). Нет информации о типе ошибки (DNS failure, connect timeout, read timeout, HTTP error). Добавить `HttpClientError` enum: `None`, `DnsError`, `ConnectError`, `ConnectTimeout`, `ReadTimeout`, `WriteTimeout`, `HttpResponseError`. Метод `send()` возвращает `HttpClientError` или `Result<IResponse, HttpClientError>`. В trading-platform BUG-07a (silent exception swallowing) — HttpBrokerGateway не может отличить «нет данных» от «сервис недоступен».
- **Файлы:** `IHttpClient.hpp`, `HttpClient.hpp`/`.cpp`
- **Тесты:** Unit-тест: каждый тип ошибки → соответствующий HttpClientError
- **Ссылка:** trading-platform BUG-07a

### SRV-26: ServerSettings не имеет дефолтных значений
- **SP:** 2
- **Модуль:** microservice-boost
- **Что:** ServerSettings требует `server.host` и `server.port` в конфиге, иначе exception. Добавить дефолты: host="0.0.0.0", port=8080. Это упростит запуск для development и testing.
- **Файлы:** `ServerSettings.hpp`/`.cpp`, `IServerSettings.hpp`
- **Тесты:** Unit-тест: config без server секции → дефолтные значения

### SRV-27: `ChainHandler` JSON injection vulnerability
- **SP:** 1
- **Модуль:** microservice-core
- **Что:** Строка `R"({"error": ")" + message + R"("})"` в ChainHandler уязвима к JSON injection, если message содержит двойные кавычки. Сейчас вызывается только с литеральной строкой, но при расширении может стать проблемой. Экранировать спецсимволы или использовать nlohmann::json для формирования.
- **Файлы:** `ChainHandler.hpp`
- **Тесты:** Unit-тест: message с `"` → escaped корректно

---

## P3 — Performance & Future

### SRV-28: Async I/O — переход на асинхронную модель
- **SP:** 13
- **Модуль:** microservice-boost
- **Что:** Заменить синхронную модель (один поток на соединение) на асинхронную на Boost.Asio. `io_context.run()` с thread pool дляhandlers, async read/write через `async_read_some()` + `async_write()`. Это масштабируемое решение: 10K+ соединений на одном процессе. BREAKING CHANGE — новая архитектура, но интерфейс IRequest/IResponse остаётся.
- **Файлы:** `BoostBeastApplication.hpp`/`.cpp` — полная переработка
- **Тесты:** Integration-тесты: concurrent connections, load testing (wrk/hey/ab), graceful shutdown
- **Зависит от:** SRV-03 (thread pool), SRV-04 (connection limit)

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
- **Что:** Текущий `HttpClient` синхронный (блокирует поток). Добавить `IAsyncHttpClient` с `sendAsync(request, callback)` — 基于 Boost.Asio async. Callback получает response или error. Connection pooling для keep-alive. В trading-pattern: circuit breaker (REL-05) и retry (REL-06) требуют async HTTP.
- **Файлы:** Новый `IAsyncHttpClient.hpp`, `AsyncHttpClient.hpp`/`.cpp`
- **Тесты:** Integration-тест: async request → callback; multiple concurrent requests

### SRV-33: WebSocket support
- **SP:** 5
- **Модуль:** microservice-boost
- **Что:** Добавить `IWebSocketHandler` и `WebSocketSession` для real-time уведомлений. В trading-platform FEAT-10 — WebSocket для order status updates. Boost.Beast поддерживает WebSocket из коробки.
- **Файлы:** Новый `IWebSocketHandler.hpp` в core, `WebSocketSession.hpp`/`.cpp` в boost
- **Тесты:** Integration-тест: connect → send message → receive message → close

### SRV-34: Разделение header-only на hpp/cpp
- **SP:** 5
- **Модуль:** microservice-boost
- **Что:** Вынести реализации из `.hpp` в `.cpp` для: BeastRequestAdapter, BeastResponseAdapter, ServerSettings, DbSettings. Оставить header-only только шаблоны и tiny classes. Ускорит компиляцию consumer-проектов.
- **Файлы:** Все `.hpp` с inline-реализациями в microservice-boost
- **Тесты:** Существующие тесты должны пройти без изменений

---

## P3 — Documentation & DX

### SRV-35: API Reference документация
- **SP:** 3
- **Модуль:** docs
- **Что:** Разделить README на отдельные документы: docs/api.md (IRequest/IResponse API), docs/routing.md (RouteMatcher, named params, wildcards), docs/middleware.md (ChainHandler, creating middleware), docs/configuration.md (ServerSettings, DbSettings, config.json), docs/deployment.md (CMake integration, FetchContent, Docker). README оставить как quick start.
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

### SRV-38: CI improvements — caching, matrix, linting
- **SP:** 3
- **Модуль:** .github
- **Что:** (1) Добавить cache для FetchContent зависимостей (Boost, DI, nlohmann_json) — сэкономит 5+ мин на каждый run. (2) Добавить matrix: build на ubuntu-latest + macOS-latest. (3) Добавить шаг clang-tidy/static-analysis. (4) Добавить шаг sanitize (AddressSanitizer, UndefinedBehaviorSanitizer).
- **Файлы:** `.github/workflows/ci.yml`, возможный `.clang-tidy`
- **Тесты:** CI проходит

---

## Сводка по Story Points

| Категория | Задач | SP |
|-----------|-------|-----|
| P1 DRY | 7 | 20 |
| P0 Critical (thread-safety, lifetime, DoS) | 4 | 13 |
| P1 Security & Reliability | 6 | 18 |
| P1 API Improvements | 5 | 19 |
| P2 Observability & DX | 5 | 12 |
| P2 Code Quality & Bugs | 7 | 15 |
| P3 Performance & Future | 7 | 45 |
| P3 Documentation & DX | 4 | 9 |
| **Итого** | **44** | **150** |