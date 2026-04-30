# TODO.md — Quick reference для AI

> **Vikunja — единственная система задач.** Все задачи создаются, управляются и закрываются в локальной Vikunja:
> <http://localhost:3456/projects/6>
> Этот файл — только читать, не редактировать задачи здесь.

---

## Открытые задачи (backlog) — 27 шт.

| ID | Код | Заголовок |
|----|-----|-----------|
| 133 | SRV-02c | BaseWebApplication — вынести boost-независимую логику |
| 134 | SRV-07b | Заголовок Allow в 405 ответе |
| 135 | SRV-10 | Config path через ENV/CLI |
| 136 | SRV-11 | Named path parameters (`:param` syntax) |
| 137 | SRV-13 | Trie-based routing |
| 138 | SRV-19 | Health check с dependency checks |
| 139 | SRV-20 | CORS middleware |
| 140 | SRV-24 | `loadJsonToEnvironment` — arrays skipped |
| 141 | SRV-28 | Async I/O — переход на асинхронную модель |
| 142 | SRV-31 | HTTPS/TLS support |
| 143 | SRV-32 | Async HTTP client |
| 144 | SRV-33 | WebSocket support |
| 145 | SRV-35 | API Reference документация |
| 146 | SRV-36 | Миграционный guide с v0.3.0 на v0.4.0 |
| 147 | SRV-44 | Миграция на UUID v7 (RFC 9562) |
| 148 | DRY-01b | Генерация main.cpp при сборке (CMake) |
| 149 | DRY-04b | Аналитика — выпиливание nlohmann/json из библиотеки |
| 150 | DRY-06 | Аналитика — оценка библиотек для внедрения |
| 151 | MSG-01 | IEventPublisher + IEventConsumer — интерфейсы сообщений ✅ |
| 152 | MSG-02 | RabbitMQAdapter — publisher/consumer + reconnect ✅ |
| 153 | MSG-03 | Circuit breaker для исходящих вызовов |
| 154 | MSG-04 | Retry policy с exponential backoff |
| 155 | MSG-05 | RabbitMQ metrics (published, received, errors) ✅ |
| 156 | DB-01 | IConnectionPool + ConnectionPool — Postgres connection pool |
| 157 | DB-02 | ITransactionExecutor + PostgresTransactionExecutor |
| 158 | DB-03 | Health check для Postgres и RabbitMQ |

*Закрытые задачи (CHANGELOG): 42 шт., `done=true` в Vikunja.*

---

## Чек-лист перед началом работы с задачей

При **каждом возвращении** к задаче (новая сессия, перезапуск, переоткрытие, взятие в работу):

1. **Перечитай постановку**: открой задачу в Vikunja, проверь описание, SP, приоритет.
2. **Прочитай комментарии**: предыдущие комментарии могут содержать прогресс, блокеры, решения.
3. **Проверь вложения** (attachments): могут быть диаграммы, логи, скриншоты.
4. **Проверь связанные задачи** (`related_tasks`): зависимости, блокеры.
5. **Проверь assignees**: убедись, что задача свободна (или assigned тебе).

---

## Где смотреть правила

- Локальные правила: `.opencode/rules/vikunja.md`
- Глобальный skill: `~/.config/opencode/skills/vikunja-tasks/SKILL.md`

---

*Последнее обновление: 2026-04-23*
