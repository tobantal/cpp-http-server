# Changelog

Все значимые изменения в проекте документируются в этом файле.

Формат основан на [Keep a Changelog](https://keepachangelog.com/ru/1.0.0/),
проект придерживается [Semantic Versioning](https://semver.org/lang/ru/).

---

## [Unreleased]

### Планируется в v0.2.0

#### Новый функционал
- `HealthCheckHandler` — дефолтный health-check эндпоинт
- `MetricsHandler` — эндпоинт для Prometheus метрик

#### Документация
- Разделение README на docs/api.md, docs/routing.md, docs/deployment.md
- TODO.md с тактическими задачами

### Планируется в будущих версиях

#### Технический долг
- Кодогенерация DI из di.json
- `HttpHandlerKey` структура вместо строкового ключа
- Разделение hpp/cpp файлов
- Named path parameters (`:orderId` синтаксис)
- Trie-based routing для эффективного матчинга

---

## [0.1.0] - 2026-01-15

### Релиз v0.1.0

Расширенная версия HTTP-сервера ом опыта реальной эксплуатации: расширены базовые интерфейсы.

#### Расширение IRequest
- `getQueryParams()` — переименование из `getParams()` для ясности
- `getQueryParam(name)` — получение параметра по имени
- `getHeader(name)` — получение заголовка по имени (case-insensitive)
- `getBearerToken()` — извлечение Bearer токена из Authorization
- `getPathPattern()` / `setPathPattern()` — поддержка path parameters
- `getPathParam(index)` — получение path parameter по индексу wildcard
- `getPathSegments()` — разбиение пути на сегменты
- `setAttribute()` / `getAttribute()` — передача данных между middleware
- `isJson()` — проверка Content-Type
- `setBody()`, `setHeader()`, `setHeaders()` — сеттеры для middleware

#### Расширение IResponse
- `getStatus()` — получение HTTP статус кода
- `getBody()` — получение тела ответа
- `getHeaders()` — получение всех заголовков
- `getHeader(name)` — получение заголовка по имени
- `setResult(code, contentType, body)` — convenience метод

#### Изменения в BoostBeastApplication
- `HandlerMatch` структура — возврат handler + pattern из findHandler()
- `HANDLER_KEY_DELIMITER` — вынос разделителя в константу
- Поддержка path parameters через `setPathPattern()`

---

## [0.0.5] - 2025-12-15

### Релиз v0.0.5

Стабильная версия с базовым функционалом HTTP-сервера.

#### Core Module (`http-server-core`)
- `IRequest` — интерфейс HTTP-запроса
- `IResponse` — интерфейс HTTP-ответа
- `IWebApplication` — базовый класс приложения (Template Method)
- `IHttpHandler` — интерфейс обработчика маршрутов
- `IHttpClient` — интерфейс HTTP-клиента
- `IEnvironment` — интерфейс конфигурации
- `RouteMatcher` — сопоставление маршрутов с wildcards (`*`)
- `Environment` — type-safe хранилище конфигурации
- `SimpleRequest` / `SimpleResponse` — реализации для тестирования

#### Boost Module (`http-server-boost`)
- `BoostBeastApplication` — HTTP-сервер на Boost.Beast/Asio
- `BeastRequestAdapter` — адаптер Beast → IRequest
- `BeastResponseAdapter` — адаптер Beast → IResponse
- `HttpClient` — синхронный HTTP-клиент
- `ServerSettings` — конфигурация хоста/порта
- `DbSettings` — параметры подключения БД

#### Тесты
- Unit-тесты для всех основных компонентов
- Покрытие: BeastRequestAdapter, BeastResponseAdapter, RouteMatcher, Settings

---

[Unreleased]: https://github.com/tobantal/cpp-http-server/compare/v0.0.5...HEAD
[0.1.0]: https://github.com/tobantal/cpp-http-server/releases/tag/v0.1.0
[0.0.5]: https://github.com/tobantal/cpp-http-server/releases/tag/v0.0.5
