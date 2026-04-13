# Архитектура cpp-http-server

## Двухмодульная структура

```
┌────────────────────────────────────────────────────────────┐
│                  microservice-core                          │
│  Интерфейсы и утилиты — header-only + RouteMatcher.cpp    │
│  Нулевые зависимости                                       │
│  IRequest, IResponse, IHttpHandler, IWebApplication       │
│  IHttpClient, IEnvironment, Environment, RouteMatcher     │
│  ChainHandler, HealthHandler, SimpleRequest, SimpleResponse│
│  ThreadSafeMap, settings/IServer...│
├────────────────────────────────────────────────────────────┤
│                  microservice-boost                         │
│  Production-реализация на Boost.Beast/Asio                │
│  Зависимости: Boost, Boost.DI, nlohmann/json             │
│  BoostBeastApplication, BeastRequestAdapter                │
│  BeastResponseAdapter, HttpClient                          │
│  settings/ServerSettings              │
└────────────────────────────────────────────────────────────┘
```

**Правило зависимостей:** microservice-boost зависит от microservice-core. microservice-core НЕ зависит от boost.

## Паттерны проектирования

### Template Method (IWebApplication)
`run()` определяет lifecycle: `loadEnvironment()` → `configureInjection()` → `start()`

### Adapter (BeastRequestAdapter, BeastResponseAdapter)
Преобразуют Boost.Beast объекты в чистые интерфейсы IRequest/IResponse.

### Chain of Responsibility (ChainHandler)
Middleware-цепочка: handlers выполняются последовательно, статус-код != 0 прерывает цепочку.

### Strategy (RouteMatcher)
Сопоставление маршрутов — можно заменить реализацию без изменения интерфейса.

### Observer/Signal (future)
Graceful shutdown через signal handler → `stop()`.

## Ключевые решения

### Status-код как сигнал цепочки
- `status == 0` = «продолжить цепочку»
- `status != 0` = «остановить цепочку»
- Это создаёт проблему для handlers, которым нужно вернуть не-200 И дать post-middleware выполниться (httpStatus-хак в consumer-проекте)

### Wildcard path parameters
- Позиционные: `getPathParam(0)` — вместо именованных `:id`
- Ограничение: один `*` = один сегмент, нет `**` для multi-segment

### Thread model (current)
- Accept loop на main thread (blocking)
- Detached thread per connection (unlimited, no pool)
- Synchronous I/O в handleSession

### HTTP Client
- Synchronous, one connection per request
- No timeouts, no connection pooling, no SSL

## Критерии качества

При каждом изменении проверять:
1. Не нарушена ли separation core/boost (core не зависит от boost)
2. Обратная совместимость — ломается ли consumer-проект?
3. Thread-safety — безопасен ли параллельный доступ к shared state?
4. Performance — нет ли O(n) bottleneck, который станет проблемой при росте?
5. Error handling — все ли пути обработки ошибок ведут к корректному ответу клиенту?