# Memory — Статус текущей работы

> Этот файл обновляется перед началом задачи, во время работы и при завершении/прерывании.
> При обрыве сессии — прочитать этот файл первым для восстановления контекста.

## Текущая задача

**Задача:** нет активной задачи
**Статус:** idle

## Контекст для восстановления

- **Ветка:** feature/v0.3.0
- **Репозиторий:** https://github.com/tobantal/cpp-http-server.git
- **Текущая версия:** v0.2.0 (v0.3.0 в разработке)
- **Consumer-проект:** cpp-trading-platform-project (33 endpoint-а, 3 микросервиса)
- **Backlog:** см. TODO.md (30 задач, 111 SP)
- **Сборка:** Ninja через CMakePresets (dev/release), 234 теста, все проходят

## Выполнено в этой сессии

1. **DRY-03** + **SRV-27** — CMake reorg + JSON injection fix
2. **SRV-17** — HttpStatus enum, getReasonPhrase(), setCookie()
3. **DRY-08 prep** — TODO task + .opencode/ директория + architecture.md rule
4. **SRV-16a** — ILogger абстракция + ConsoleLogger + NullLogger + TestLogger
   - Все 38 std::cerr/cout в BoostBeastApplication и HttpClient заменены на logger_->log()
   - ChainHandler использует logger_ вместо std::cerr
   - IWebApplication: setLogger()/getLogger() с NullLogger по умолчанию
   - HttpClient: setLogger()
   - LogLevel enum : uint8_t, logLevelToString()
   - 10 новых тестов
5. **SRV-34** — поглощена DRY-08 (hpp/cpp split — часть архитектурного порядка)
6. **Style rule** — hpp = declaration + docs only, .cpp = implementation (template exception noted)

## Правила, добавленные в этой сессии

- `.opencode/rules/architecture.md` — структура файлов, интерфейсы vs реализации, доменные папки
- `.opencode/rules/cpp-style.md` — hpp = declaration + docs, .cpp = implementation, template exception
- `.claude/CLAUDE.md` — redirect на `.opencode/OPENCODE.md`

## Следующие задачи по приоритету (из TODO.md)

1. **SRV-02c** (SP:5, P0) — BaseWebApplication абстракция
2. **SRV-04** (SP:5, P0) — Thread pool вместо unlimited threads (DoS)
3. **SRV-08** (SP:5, P1) — Graceful shutdown (IShutdown + ShutdownManager)
4. **DRY-07** (SP:3, P1) — JsonHelper (serialize/deserialize DTO)
5. **DRY-08** (SP:3, P1) — Архитектурный порядок файлов (error/, handler/, util/)