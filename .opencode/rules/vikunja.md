# Правила работы с задачами (Vikunja)

> **Этот файл — локальные правила для AI-разработчика проекта cpp-http-server.**
> Общий skill по Vikunja API: `~/.config/opencode/skills/vikunja-tasks/SKILL.md`

---

## Единственный источник правды

- **Vikunja** (локальная инсталляция: http://localhost:3456) — единственная система для:
  - Создания задач
  - Взятия в работу
  - Обновления статуса и прогресса
  - Закрытия задач
  - Комментариев и трассировки
- **TODO.md** в репозитории — только quick-reference список ID задач для быстрого поиска.
  - НЕ редактируй задачи в TODO.md.
  - НЕ создавай задачи через TODO.md.
  - Обновляй TODO.md только при массовом переносе релиза (когда все задачи релиза закрыты).

---

## Проект в Vikunja

| Проект | ID | Назначение |
|--------|-----|-----------|
| cpp-http-server | **Project 6** | Все задачи по серверной библиотеке (CHANGELOG + TODO) |

---

## Жизненный цикл задачи

### 1. Просмотр актуальных задач

- Открыть Kanban «To-Do» в Project 6: http://localhost:3456/projects/6
- Все открытые задачи (`done=false`) уже в колонке To-Do.
- Фильтр по приоритету: P0 → P1 → P2 → P3.

### 2. Чек-лист перед началом работы с задачей

При **каждом возвращении** к задаче (новая сессия, перезапуск, переоткрытие, взятие в работу):

1. **Перечитай постановку**: открой задачу в Vikunja (`GET /api/v1/tasks/${TASK_ID}`), проверь описание, SP, приоритет.
2. **Прочитай комментарии** (`GET /api/v1/tasks/${TASK_ID}/comments`): предыдущие комментарии могут содержать прогресс, блокеры, решения.
3. **Проверь вложения** (attachments): могут быть диаграммы, логи, скриншоты.
4. **Проверь связанные задачи** (`related_tasks`): зависимости, блокеры (если задача `blocked` — не брать в работу).
5. **Проверь assignees**: убедись, что задача свободна (или assigned тебе).
6. **Прочитай описание заново**: постановка могла обновиться с последнего раза.

### 3. Взятие задачи в работу

```bash
# 1. Прочитать задачу (для получения текущего состояния)
curl -s -H "Authorization: Bearer ${TOKEN}" \
  "http://localhost:3456/api/v1/tasks/${TASK_ID}" | jq

# 2. Добавить комментарий «Взял в работу»
curl -s -X PUT "http://localhost:3456/api/v1/tasks/${TASK_ID}/comments" \
  -H "Authorization: Bearer ${TOKEN}" \
  -H "Content-Type: application/json" \
  -d '{"comment": "Взял в работу"}' | jq

# 3. Переместить в «Doing» (Kanban)
# Добавить в bucket Doing (ID зависит от проекта — см. /projects/6/views)
curl -s -X POST "http://localhost:3456/api/v1/projects/6/views/${VIEW_ID}/buckets/${DOING_BUCKET_ID}/tasks" \
  -H "Authorization: Bearer ${TOKEN}" \
  -H "Content-Type: application/json" \
  -d "{\"task_id\": ${TASK_ID}}" | jq

# 4. Обновить start_date (через read-first-write-all)
# См. раздел «Обновление задачи» ниже
```

### 3. Обновление прогресса

```bash
# Прочитать текущее состояние (ОБЯЗАТЕЛЬНО)
CURRENT=$(curl -s -H "Authorization: Bearer ${TOKEN}" \
  "http://localhost:3456/api/v1/tasks/${TASK_ID}")

# Обновить percent_done = 0.5 (50%)
# Включить ВСЕ поля из CURRENT, иначе они сбросятся в ноль
title=$(echo "$CURRENT" | jq -r '.title')
desc=$(echo "$CURRENT" | jq -r '.description')
priority=$(echo "$CURRENT" | jq -r '.priority')
hex_color=$(echo "$CURRENT" | jq -r '.hex_color // empty')
start=$(echo "$CURRENT" | jq -r '.start_date')
end=$(echo "$CURRENT" | jq -r '.end_date')
bucket=$(echo "$CURRENT" | jq -r '.bucket_id')

curl -s -X POST "http://localhost:3456/api/v1/tasks/${TASK_ID}" \
  -H "Authorization: Bearer ${TOKEN}" \
  -H "Content-Type: application/json" \
  -d "{
    \"title\": $(echo "$title" | jq -Rs .),
    \"description\": $(echo "$desc" | jq -Rs .),
    \"priority\": $priority,
    \"hex_color\": $(echo "$hex_color" | jq -Rs .),
    \"start_date\": \"$start\",
    \"end_date\": \"$end\",
    \"bucket_id\": $bucket,
    \"done\": false,
    \"percent_done\": 0.5
  }" | jq '{id, title, percent_done}'
```

### 4. Закрытие задачи

```bash
# Переместить в «Done»
curl -s -X POST "http://localhost:3456/api/v1/projects/6/views/${VIEW_ID}/buckets/${DONE_BUCKET_ID}/tasks" \
  -H "Authorization: Bearer ${TOKEN}" \
  -H "Content-Type: application/json" \
  -d "{\"task_id\": ${TASK_ID}}" | jq

# Обновить: done=true, percent_done=1.0, end_date=now
# Через read-first-write-all с end_date=$(date -u +"%Y-%m-%dT%H:%M:%SZ")
```

---

## CRITICAL: read-first-write-all

Vikunja API v2 — **POST обновление сбрасывает ВСЕ неуказанные поля** в ноль/пустые значения.

| Поле (не указано) | Что произойдёт |
|-------------------|----------------|
| `title` | Пустая строка |
| `description` | Пустая строка |
| `priority` | 0 (None) |
| `hex_color` | `""` |
| `start_date` | `"0001-01-01T00:00:00Z"` |
| `end_date` | `"0001-01-01T00:00:00Z"` |
| `bucket_id` | 0 (задача «вылетает» из Kanban колонки, возвращается в To-Do по умолчанию) |
| `done` | `false` |
| `percent_done` | 0 |

**Всегда** делай GET перед POST и включай все поля из GET-ответа.

---

## CRITICAL: bucket_id vs Kanban bucket

- `bucket_id` в теле задачи — **legacy поле**. Отображение в UI может не совпадать с реальной Kanban-колонкой.
- **Реальное перемещение** между To-Do / Doing / Done делается через:
  `POST /projects/{pid}/views/{vid}/buckets/{bid}/tasks` с `{"task_id": N}`
- После перемещения через bucket API **сохраняй текущий `bucket_id`** (или передавай `0`) — Vikunja может сбросить Kanban-ассоциацию.

---

## Приоритеты

| P-level | Приоритет Vikunja (int) | Значение | Когда брать |
|---------|------------------------|----------|-------------|
| P0 (Critical) | 5 | DO NOW | Блокирующие, срочные |
| P1 (Urgent) | 4 | Urgent | Текущий спринт |
| P2 (High) | 3 | High | Ближайший спринт |
| P3 (Low) | 1 | Low | Backlog |
| P4 | 1 | Low | Backlog / nice-to-have |

---

## Язык и формат

- **Заголовки задач:** Русский.
- **ID задачи** (`SRV-01`, `DRY-02`) и технические термины: **не переводить**.
- **Описание:** plain text, дефисные списки `-`. **Не использовать markdown-заголовки `###`** — Vikunja рендерит их криво.
- **Комментарии:** удалять спам/промежуточные комментарии. Оставлять «Взял в работу», «Сделано», «Заблокировано задачей X».

---

## Миграция CHANGELOG → Vikunja

- CHANGELOG.md **больше не ведётся** в репозитории.
- Все выполненные задачи перенесены в Vikunja Project 6 как **закрытые** (`done=true`).
- Новые задачи (следующих релизов) создаются напрямую в Vikunja, **не в TODO.md/CHANGELOG.md**.

---

## Quick Commands

```bash
# Login
export TOKEN=$(curl -s -X POST "http://localhost:3456/api/v1/login" \
  -H "Content-Type: application/json" \
  -d '{"username":"anton","password":"vikunja2024"}' | jq -r '.token')

# List open tasks in To-Do bucket
curl -s -H "Authorization: Bearer $TOKEN" \
  "http://localhost:3456/api/v1/projects/6/tasks?per_page=50&bucket=19" | jq '.[] | {id, title, priority}'

# Get task details
curl -s -H "Authorization: Bearer $TOKEN" \
  "http://localhost:3456/api/v1/tasks/${TASK_ID}?expand=buckets" | jq

# Add comment
curl -s -X PUT "http://localhost:3456/api/v1/tasks/${TASK_ID}/comments" \
  -H "Authorization: Bearer $TOKEN" \
  -H "Content-Type: application/json" \
  -d '{"comment": "Комментарий"}' | jq
```

---

*Последнее обновление: 2026-04-23*
