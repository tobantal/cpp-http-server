# Security — MANDATORY правила для HTTP-сервера

> Нарушение этих правил — критический баг. Проверять при каждом ревью.

## HTTP-сервер безопасность

### Request validation
- **Request body size limit** — обязательный лимит (по умолчанию 1MB, конфигурируемый). Без лимита — DoS через огромный body.
- **Request timeout** — обязательный таймаут на чтение (по умолчанию 30s). Без таймаута — slowloris DoS.
- **URL decoding** — path и query params декодировать, проверять на path traversal (`..`)
- **Header size limit** — лимит на размер заголовков (8KB по умолчанию)

### Thread-safety
- **`handlers_` map** — доступ из detached threads → нужна синхронизация или иммутабельность после start()
- **`state_`** — `std::atomic<ServerState>`, состояния: NotStarted, Running, Stopped. Доступ из main thread + signal handler. `registerHandler()` разрешён только в NotStarted. `stop()` использует `compare_exchange(Running, Stopped)`.
- **Request/Response objects** — создаются per-session, не разделяются между потоками. OK.
- **Detached threads** — lifetime hazard: `this` может быть уничтожен пока thread работает. Решение: thread pool + join при stop().

### Resource limits
- **Max connections** — лимит на число одновременных соединений (без лимита = DoS через thread exhaustion)
- **Thread pool** — заменить detached threads на пул фиксированного размера
- **Keep-alive** — лимит на число запросов на одном соединении (по умолчанию 100)

### Response security
- Не включать серверную версию/ОС в заголовки ответа
- Не включать stack traces в 500 ошибки (только `{"error": "Internal server error"}`)
- Установить `X-Content-Type-Options: nosniff`
- Установить `X-Frame-Options: DENY`

## HTTP-клиент безопасность

- **Таймауты** — connect, read, write (без таймаутов — cascade hang)
- **SSL/TLS** — для HTTPS endpoints (verify cert, не skip)
- **Нет plaintext credentials** в URL или заголовках, кроме явно конфигурируемых
- **Redirect limit** — не следовать редиректам бесконечно

## Configuration security

- **Config path** — не хардкодить `config.json`, конфигурировать через ENV или CLI
- **Secrets** — не хранить в config.json, только в ENV / K8s Secrets
- **Port** — не биндиться на 0.0.0.0 без явной настройки (default: localhost)