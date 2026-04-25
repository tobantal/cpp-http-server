# Core Handlers

Middleware and utility handlers provided by microservice-core.

---

## ChainHandler

Middleware chain — executes all handlers sequentially.

```cpp
#include "ChainHandler.hpp"
```

### Usage

```cpp
// Create with multiple handlers (variadic)
auto chain = std::make_shared<ChainHandler>(handler1, handler2, handler3);

// With logger and error handler
auto chain = std::make_shared<ChainHandler>(logger, errorHandler, handler1, handler2);
```

### Behavior

1. Executes **all** handlers in order (does NOT break on status)
2. If handler throws `HttpError` — chain stops, delegates to error handler
3. If handler throws `std::exception` — chain stops, returns 500
4. After all handlers: if `status < 100 || status >= 600` → error (no handler set valid status)
5. Trace ID: extracts from `X-Trace-ID` header or generates UUID v4

### Error Handling Flow

```
Handler throws HttpError → errorHandler->handleError() → return
Handler throws std::exception → HttpError(500) → errorHandler->handleError() → return
All handlers complete with valid status (100-599) → continue
All handlers complete with invalid status → HttpError(500)
```

### Status Code Rules

- All handlers run regardless of status
- Valid HTTP status: 100-599
- Invalid status after chain: treated as error (500)

---

## HealthHandler

Simple health check endpoint.

```cpp
#include "HealthHandler.hpp"
```

### Response

```json
{"status": "UP"}
```

### Registration

```cpp
registerEndpoint("GET", "/health", std::make_shared<HealthHandler>());
```

---

## MetricsHandler

Prometheus metrics endpoint.

```cpp
#include "MetricsHandler.hpp"
```

### Registration

```cpp
auto metrics = std::make_shared<MetricsCollector>();
registerEndpoint("GET", "/metrics", std::make_shared<MetricsHandler>(metrics));
```

### Output

Prometheus text format with counters, gauges, histograms.

---

## MetricsObserverHandler

Decorator that wraps handler chain and records metrics.

```cpp
#include "MetricsObserverHandler.hpp"
```

### Usage

```cpp
auto observer = std::make_shared<MetricsObserverHandler>(
    chainHandler,   // inner handler
    metrics,        // MetricsCollector
    "my-service"    // service name for metrics labels
);
```

### Metrics Recorded

- `http_requests_total{method, path, status}` — counter
- `http_request_duration_seconds{method, path}` — histogram

---

## HttpErrorSender

Default error handler — JSON error responses.

```cpp
#include "HttpErrorSender.hpp"
```

### Response Format

```json
{"error": "Not found", "code": 404}
```

---

## See Also

- [Interfaces](interfaces.md) — IHttpHandler, IHttpErrorHandler
- [Utilities](utilities.md) — UuidGenerator, Timer
- [Boost Application](../boost/application.md) — BoostBeastApplication