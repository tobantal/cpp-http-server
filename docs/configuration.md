# Configuration

cpp-http-server supports two configuration sources: **environment variables** (higher priority) and **config.json** (fallback).

## Priority

```
ENV variable > config.json key > default value
```

- `ServerSettings` checks ENV first, falls back to `env->get("server.*")` from config.json, then uses hardcoded defaults.
- `HttpClient` reads only ENV variables for timeouts (no config.json fallback).

## Server Settings

| ENV Variable | config.json Key | Type | Default | Description |
|---|---|---|---|---|
| `SERVER_HOST` | `server.host` | string | `0.0.0.0` | Bind address |
| `SERVER_PORT` | `server.port` | int | `8080` | Listen port |
| `SERVER_MAX_REQUEST_BODY_SIZE` | `server.maxRequestBodySize` | size_t | `1048576` | Max request body in bytes (1 MB). Returns 413 if exceeded |
| `SERVER_READ_TIMEOUT_MS` | `server.readTimeoutMs` | int | `30000` | Read timeout in ms |
| `SERVER_WRITE_TIMEOUT_MS` | `server.writeTimeoutMs` | int | `30000` | Write timeout in ms |
| `SERVER_MAX_CONNECTIONS` | `server.maxConnections` | size_t | `0` | Max concurrent connections. 0 = unlimited. Returns 503 if exceeded |
| `SERVER_MAX_REQUESTS_PER_CONNECTION` | `server.maxRequestsPerConnection` | size_t | `100` | Max requests per keep-alive connection. Connection closes after limit |

## HTTP Client Settings (outgoing requests)

| ENV Variable | Type | Default | Description |
|---|---|---|---|
| `HTTP_CLIENT_CONNECT_TIMEOUT_MS` | int | `5000` | Connect timeout in ms (5s) |
| `HTTP_CLIENT_READ_TIMEOUT_MS` | int | `30000` | Read timeout in ms (30s) |
| `HTTP_CLIENT_WRITE_TIMEOUT_MS` | int | `30000` | Write timeout in ms (30s) |

## Circuit Breaker Settings

| ENV Variable | Type | Default | Description |
|---|---|---|---|
| `CIRCUIT_BREAKER_FAILURE_THRESHOLD` | int | `5` | Number of failures before opening the circuit |
| `CIRCUIT_BREAKER_FAILURE_WINDOW_SECONDS` | int | `30` | Time window in seconds to count failures |
| `CIRCUIT_BREAKER_HALF_OPEN_TIMEOUT_SECONDS` | int | `60` | Time in seconds before transitioning from OPEN to HALF_OPEN |

**State Machine:**
```
CLOSED ──(failures >= threshold)──> OPEN
  ↑                                │
  │                                │
(success)                      (timeout)
  │                                │
  │                                ▼
  └────(success)──────────── HALF_OPEN
```

| State | Behavior |
|---|---|
| CLOSED | Normal operation. Requests pass through. Failures counted. |
| OPEN | Fail-fast mode. Requests blocked. After timeout → HALF_OPEN. |
| HALF_OPEN | Probe mode. One request allowed. Success → CLOSED, Failure → OPEN. |

**Usage Example:**
```cpp
#include "circuit/CircuitBreaker.hpp"
#include "circuit/CircuitBreakerSettings.hpp"

// Via settings interface
auto settings = std::make_shared<CircuitBreakerSettings>("CIRCUIT_BREAKER_");
CircuitBreaker cb(settings);

// Or via ENV prefix
CircuitBreaker cb("HTTP_CLIENT_");  // reads HTTP_CLIENT_FAILURE_THRESHOLD, etc.

// In your service code
if (!cb.allowRequest()) {
    throw ServiceUnavailableException("Circuit is open");
}
try {
    auto result = httpClient->send(request, response);
    cb.recordSuccess();
    return result;
} catch (...) {
    cb.recordFailure();
    throw;
}
```

## config.json Example

```json
{
  "server": {
    "host": "0.0.0.0",
    "port": 8080,
    "maxRequestBodySize": 1048576,
    "readTimeoutMs": 30000,
    "writeTimeoutMs": 30000,
    "maxConnections": 0,
    "maxRequestsPerConnection": 100
  }
}
```

Load config via `BoostBeastApplication::loadConfig(path)` — it reads JSON and populates `IEnvironment`.

## Adding New Settings

When adding a new ENV variable or config key:

1. Add the constant with default in `ServerSettings.hpp` or `HttpClient.hpp`
2. Add reading logic (ENV first, then config.json fallback)
3. Update `.env.example` with the new variable
4. Update this document (`docs/configuration.md`)