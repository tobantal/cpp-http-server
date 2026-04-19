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
- **Backlog:** см. TODO.md (38 задач, 130 SP)
- **Сборка:** Ninja через CMakePresets (dev/release), 208 тестов, все проходят

## Выполнено в этой сессии

1. **DRY-03** — Реорганизация CMake: find_package приоритет + FetchContent fallback (опция `CPP_HTTP_SERVER_FETCH_DEPS=ON`)
2. **Boost.DI удалён** — не использовался ни в одном файле (0 include'ов), убран из CMakeLists.txt
3. **GoogleTest дедуплицирован** — единый FetchContent в корневом CMake
4. **Версия проекта** — `project(cpp-http-server VERSION 0.3.0)`
5. **SRV-27** — `StringUtils::escapeJson()`: экранирование `"`, `\`, `\n`, `\r`, `\t`, control chars → закрывает JSON injection в ChainHandler и BoostBeastApplication
6. 12 новых тестов (10 escapeJson + 2 ChainHandler JSON injection)

## Правила, добавленные в этой сессии

- (нет новых правил)

## Следующие задачи по приоритету (из TODO.md)

1. **SRV-02c** (SP:5, P0) — BaseWebApplication абстракция
2. **SRV-04** (SP:5, P0) — Thread pool вместо unlimited threads (DoS)
3. **SRV-08** (SP:5, P1) — Graceful shutdown (IShutdown + ShutdownManager)
4. **DRY-07** (SP:3, P1) — JsonHelper (serialize/deserialize DTO)
5. **SRV-11** (SP:5, P1) — Named path parameters (`:param` синтаксис)