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


### Выполнено в v0.3.0 (feature/v0.3.0)


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
