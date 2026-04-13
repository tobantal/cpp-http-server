# cpp-http-server — Инструкции для Claude Code

## Проект

C++17 библиотека HTTP-сервера и HTTP-клиента для микросервисной архитектуры. Репозиторий: https://github.com/tobantal/cpp-http-server

Два модуля:
- **microservice-core** — чистые интерфейсы (IRequest, IResponse, IHttpHandler, IWebApplication, IHttpClient, IEnvironment, RouteMatcher, ChainHandler, HealthHandler)
- **microservice-boost** — production-реализация на Boost.Beast/Asio (BoostBeastApplication, BeastRequestAdapter, BeastResponseAdapter, HttpClient, ServerSettings, DbSettings)

**Цель:** Упростить, улучшить, сделать надёжной и быстрой библиотеку для биржевой торговой платформы (consumer-проект) и других микросервисов на C++17. Ориентироваться на production-ready качество: thread-safety, безопасность, производительность, удобство API.

## Consumer-проект (trading-platform)

Библиотека используется в торговой платформе (cpp-trading-platform-project):
- 3 микросервиса: auth (8081), trading (8082), broker (8083)
- 33 HTTP-endpoint-а, 4 middleware, ~36 handler-классов
- Архитектура: Hexagonal (Ports & Adapters), Boost.DI
- Известные pain points в consumer-проекте, требующие доработки библиотеки: httpStatus-хак (REF-09), wildcard path params (FUT-01/FUT-02), O(n) роутинг (FUT-01), отсутствие таймаутов в HttpClient

## Сборка и тесты

CMake-сборка (C++17, CMake 3.14+):

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build
cd build && ctest --verbose
```

Сборка с coverage:
```bash
cmake -B build -DCMAKE_BUILD_TYPE=Debug -DCMAKE_CXX_FLAGS="--coverage -g -O0"
cmake --build build
cd build && ctest --verbose
gcovr --root . --exclude '.*CMakeFiles/.*' --exclude '.*/tests/.*' --exclude '.*_deps/.*' --html-details coverage.html --print-summary
```

## Процесс работы

1. **Перед началом задачи** — прочитать `memory.md` для восстановления контекста
2. Задачи брать из `TODO.md` по приоритету (P0 → P1 → P2 → P3)
3. Перед правкой кода — прочитать файл целиком, понять контекст
4. Соблюдать существующую архитектуру (core interfaces → boost implementations)
5. **Каждая задача — отдельный коммит в GitHub**
6. Перед коммитом — ещё раз проверить: компиляция, тесты, соответствие стандарту
7. После коммита — убедиться, что CI проходит (GitHub Actions: cmake build + ctest + coverage)
8. Не переходить к следующей задаче, пока текущая не верифицирована полностью
9. Не упоминать ИИ в тексте коммитов
10. **После завершения/прерывания задачи** — обновить `memory.md`
11. **Backward compatibility** — новые версии библиотеки не должны ломать существующий код consumer-проектов. Если breaking change неизбежен — описать миграцию в CHANGELOG

## Где искать идеи и исследования

- **`docs/TAGS.md`** — Git tagging workflow
- **`CHANGELOG.md`** — история версий библиотеки
- **`README.md`** — документация API и примеры использования

## Правила (обязательно к исполнению)

- **[cpp-style.md](rules/cpp-style.md)** — C++17 стандарт, naming, память, Boost.DI, header organization
- **[error-handling.md](rules/error-handling.md)** — исключения vs optional vs коды возврата, паттерны
- **[security.md](rules/security.md)** — input validation, thread-safety, resource limits
- **[testing.md](rules/testing.md)** — GTest, mock-и, именование, покрытие

## Проектные справочники

- **[architecture.md](architecture.md)** — архитектура библиотеки, паттерны, расширяемость

## Запрещено

- Менять C++17 на другой стандарт без согласования
- Нарушать separation of interfaces/implementations (core не зависит от boost)
- Добавлять зависимости без крайней необходимости
- Добавлять зависимости в microservice-core (header-only, нулевые зависимости)
- Пропускать падающие тесты
- Ломать backward compatibility без версии и описание миграции