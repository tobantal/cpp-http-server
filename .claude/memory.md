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
- **Backlog:** см. TODO.md (39 задач, 133 SP)
- **Сборка:** Ninja через CMakePresets (dev/release), 196 тестов, все проходят

## Выполнено в этой сессии

1. **SRV-02b** (уже был закоммичен, верифицирован) — `enum class ServerState : uint8_t` вместо двух `atomic<bool>`
2. **cpp-style.md** — добавлены правила: padding/ordering полей, `enum class : uint8_t`, уточнён hpp/cpp порог (150 строк) + таблица нарушений SRV-34
3. **ServerState : uint8_t** — применено новое правило к единственному enum в проекте
4. **Ninja presets** — `CMakePresets.json` (dev/release) + `.vscode/settings.json`
5. **SRV-39** — JsonValidator middleware (JSON parse check + Content-Type validation), 15 тестов
6. **TODO cleanup** — удалены выполненные задачи SRV-02b, SRV-06, SRV-07
7. **SRV-22** — BeastRequestAdapter::getPort() из socket.local_endpoint(), 3 теста
8. **SRV-06b** — HttpClient connect timeout: async_connect + steady_timer, ENV HTTP_CLIENT_CONNECT_TIMEOUT_MS (default: 5000ms), 4 теста

## Правила, добавленные в этой сессии

- **Порядок полей в структурах** — по убыванию размера (8→4→2→1 байт), минимизация padding
- **enum class : uint8_t** — всегда для доменных перечислений с малым количеством значений
- **Header-only vs hpp/cpp** — порог 150 строк inline-реализации, чистые интерфейсы без ограничения

## Следующие задачи по приоритету (из TODO.md)

1. **SRV-02c** (SP:5) — BaseWebApplication абстракция
2. **SRV-04** (SP:5) — Thread pool вместо unlimited threads (DoS-уязвимость)
3. **SRV-08** (SP:5) — Graceful shutdown (IShutdown + ShutdownManager)
4. **DRY-07** (SP:3) — deserialize<T>(body) — JSON string → типизированный объект
5. **SRV-11** (SP:5) — Named path parameters (`:param` синтаксис)