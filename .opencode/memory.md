# Memory — Статус текущей работы

> Этот файл обновляется перед началом задачи, во время работы и при завершении/прерывании.
> При обрыве сессии — прочитать этот файл первым для восстановления контекста.

## Текущая задача

**Задача:** нет активной задачи
**Статус:** idle

## Контекст для восстановления

- **Ветка:** feature/v0.3.0
- **Репозиторий:** https://github.com/tobantal/cpp-http-server.git
- **Текущая версия:** v0.3.0 (в разработке, CMake `project(... VERSION 0.3.0)`)
- **Consumer-проект:** cpp-trading-platform-project (33 endpoint-а, 3 микросервиса)
- **Backlog:** см. TODO.md (21 задача, 83 SP)
- **Сборка:** Ninja через CMakePresets (dev/release), 284 теста, все проходят
- **Архитектура:** hexagonal (ports/adapters/domain/application) — рефакторинг завершён

## Выполнено в предыдущих сессиях

1. **DRY-03** + **SRV-27** — CMake reorg + JSON injection fix
2. **SRV-17** — HttpStatus enum, getReasonPhrase(), setCookie()
3. **DRY-08 prep** — TODO task + .opencode/ директория + architecture.md rule
4. **SRV-16a** — ILogger абстракция + ConsoleLogger + NullLogger + TestLogger
5. **SRV-34** — поглощена DRY-08 (hpp/cpp split — часть архитектурного порядка)
6. **Style rule** — hpp = declaration + docs only, .cpp = implementation
7. **DRY-07** — JsonSerializer/deserialize + JsonParseError
8. **DRY-08** — архитектурная структура файлов (error/, handler/, util/, ports/, adapters/, domain/, application/) + forwarding headers
9. **SRV-04** — Connection limit, 503 на превышение
10. **SRV-09** — HttpClientError enum + decomposed send + read/write timeouts
11. **SRV-14** — URL-декодирование в getPath() и getQueryParams()
12. **SRV-16b** — Logger refactoring: constructor injection, remove setLogger/getLogger
13. **SRV-18** — Tracing middleware (X-Trace-ID в ChainHandler)
14. **SRV-38** — CI improvements: caching, matrix, linting, sanitizers
15. **Hexagonal refactor** — ports, adapters, domain, application структура

## Выполнено в текущей сессии

1. **SRV-37** — Versioning и semver для библиотеки
   - Исправлен баг в `VersionTest.VersionStringFormat` (некорректное условие `std::string::npos == false ? ... : 0` → `EXPECT_NE(version.find('.'), std::string::npos)`)
   - 284/284 тестов проходят
   - CHANGELOG.md: добавлена секция SRV-37
   - TODO.md: SRV-37 отмечена завершённой, сводка обновлена

## Правила

- `.opencode/rules/architecture.md` — hexagonal структура, интерфейсы vs реализации, доменные папки
- `.opencode/rules/cpp-style.md` — hpp = declaration + docs, .cpp = implementation, template exception

## Следующие задачи по приоритету (из TODO.md)

1. **SRV-02c** (SP:5, P0) — BaseWebApplication абстракция
2. **SRV-08** (SP:5, P1) — Graceful shutdown (IShutdown + ShutdownManager)
3. **SRV-11** (SP:5, P1) — Named path parameters (`:param` syntax)
4. **SRV-10** (SP:2, P1) — Config path через ENV/CLI
5. **DRY-01** (SP:2, P1) — Исследование: включить main.cpp в поставку библиотеки