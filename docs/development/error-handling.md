# Error Handling Guidelines

Mandatory error handling rules. Violation = direct bug. Review at every PR.

---

## Exceptions vs Optional vs Return Codes

- **Exceptions** — for unexpected errors (config not found, boost::system::error, unexpected state)
- **`std::optional<T>`** — for "may not have result" (findHandler, getQueryParam, getHeader)
- **Return codes** — `bool` for success/failure operations (HttpClient::send)
- **Don't use exceptions for flow control**

---

## HTTP Server Error Rules

### BoostBeastApplication

- JSON parse errors in `loadEnvironment` → `std::runtime_error` (crash on start — OK, config is mandatory)
- Errors in `handleSession` (`beast::system_error` end_of_stream) → silently close connection
- Unknown exceptions in handler → 500 `{"error": "Internal server error"}` + log to stderr
- `findHandler` errors (route not found) → 404 `{"error": "Not found"}`
- Route errors (method not supported) → 405 (when implemented)

### HttpClient

- Connection/resolution errors → `return false`, response.setStatus(500)
- Don't swallow exceptions silently — log before returning
- Future: add error code enum (ConnectionFailed, Timeout, DnsError)

### ChainHandler

- All handlers run regardless of status (no early break)
- If all handlers complete with status 100-599 → continue
- If status < 100 or >= 600 after chain → 500 Internal Server Error
- JSON in error must not contain injections (escape user input)

---

## Forbidden

- `catch (...) { /* empty */ }` — **FORBIDDEN**. At least log it.
- Log and swallow without returning error — only if error is truly non-critical
- Use exceptions for expected paths (no route → 404, not exception; but no config → exception)

---

## Error Handling Pattern for BoostBeastApplication

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