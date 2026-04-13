# Error Handling — MANDATORY правила

> Нарушение этих правил — прямой баг. Проверять при каждом ревью.

## Исключения vs optional vs коды возврата

- **Исключения** — для непредвиденных ошибок (config not found, boost::system::error, unexpected state)
- **`std::optional<T>`** — для «может не быть результата» (findHandler, getQueryParam, getHeader)
- **Коды возврата** — `bool` для операций success/failure (HttpClient::send)
- **Не использовать исключения для flow control**

## Правила для HTTP-сервера

### BoostBeastApplication
- Ошибки парсинга JSON в `loadEnvironment` → `std::runtime_error` (краш при старте — ок, конфиг обязателен)
- Ошибки в `handleSession` (`beast::system_error` end_of_stream) → молча закрыть соединение
- Неизвестные исключения в handler → 500 `{"error": "Internal server error"}` + логирование в stderr
- Ошибки `findHandler` (не найден маршрут) → 404 `{"error": "Not found"}`
- Ошибки маршрута (метод не поддерживается) → 405 (когда будет реализовано)

### HttpClient
- Ошибки соединения/резолвинга → `return false`, response.setStatus(500)
- Не глушить исключения молча — логировать перед возвратом
- В перспективе: добавить error code enum (ConnectionFailed, Timeout, DnsError)

### ChainHandler
- Status 0 = continue, status != 0 = stop — это контракт, не нарушать
- Если цепочка завершилась со status 0 → 500 Internal Server Error
- JSON в ошибке не должен содержать инъекций (экранировать user input)

## Запрещено

- `catch (...) { /* empty */ }` — ЗАПРЕЩЕНО. Хотя бы логировать.
- Логировать и глушить без возврата ошибки — только если ошибка действительно некритична
- Использовать исключения для expected path (нет маршрута → 404, не исключение; а вот нет конфига → исключение)

## Паттерн для BoostBeastApplication

```cpp
void handleRequest(IRequest& req, IResponse& res) {
    auto match = findHandler(req.getMethod(), req.getPath());
    if (!match) {
        res.setResult(404, "application/json", R"({"error":"Not found"})");
        return;
    }
    try {
        req.setPathPattern(match->pattern);
        match->handler->handle(req, res);
    } catch (const std::exception& e) {
        std::cerr << "Handler error: " << e.what() << std::endl;
        res.setResult(500, "application/json", R"({"error":"Internal server error"})");
    }
}
```