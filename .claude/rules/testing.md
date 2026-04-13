# Testing — правила для cpp-http-server

> Применимо к библиотеке HTTP-сервера с GTest.

## Фреймворк и расположение

- Google Test (GTest) — основной фреймворк
- microservice-core тесты → `microservice-core/tests/`
- microservice-boost тесты → `microservice-boost/tests/`
- CI: GitHub Actions (cmake build + ctest + coverage)

## Именование

- `TestSuiteName.TestScenario_ExpectedResult`
- Пример: `RouteMatcherTest.WildcardMiddle_MatchesPath`
- Пример: `BeastRequestAdapter.GetPathParam_ReturnsValue`
- Пример: `BoostBeastApplication.HandleRequest_Returns404ForUnknownRoute`

## Что покрывать

### Core module
- IRequest/IResponse — через SimpleRequest/SimpleResponse
- RouteMatcher — exact match, wildcards, edge cases (trailing slash, empty path, multiple wildcards)
- ChainHandler — short-circuit, full chain, empty chain, single handler
- Environment — type access, missing keys, defaults

### Boost module
- BeastRequestAdapter — все методы IRequest через реальный Beast-запрос
- BeastResponseAdapter — все методы IResponse
- BoostBeastApplication — lifecycle (start/stop), routing, path params, method not allowed
- HttpClient — connect, timeout (with mock server), error handling
- ServerSettings — ENV приоритет, config.json fallback, дефолты

### Integration
- Запуск реального сервера → HTTP-запрос → проверка ответа (как в HttpClientTest)

## Mock-подход

- SimpleRequest/SimpleResponse — основные test doubles для IRequest/IResponse
- Для тестирования HttpClient поднимать реальный сервер на случайном порту (как в HttpClientTest)
- Для тестирования BoostBeastApplication поднимать сервер с тестовыми handler-ами

## Перед коммитом — обязательно

1. **Unit-тесты** — `cd build && ctest --verbose` — все должны пройти
2. **Coverage** — проверить что новые строки покрыты
3. **Backward compatibility** — consumer-проект (trading-platform) должен собираться без изменений

## Запрещено

- Пропускать падающие тесты
- Использовать `sleep()` в тестах (использовать моки для времени, или короткие таймауты)
- Hardcoded порты в тестах (использовать порт 0 — ОС назначает свободный, или случайный из диапазона)

## Недостатки текущих тестов (улучшить)

- Нет тестов для concurrency/thread-safety (ThreadSafeMap и handlers_ map)
- Нет тестов для timeout-scenarios в HttpClient
- Нет тестов для request body size limits
- Нет тестов для malformed HTTP requests
- Нет stress/load тестов