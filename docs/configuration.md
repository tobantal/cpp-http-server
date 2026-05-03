# Configuration

cpp-http-server supports two configuration sources: **environment variables** (higher priority) and **config.json** (fallback).

## Priority

```
CLI argument (--config/-c) > ENV variable > config.json key > default value
```

For configuration file path:
```
--config <path> > CONFIG_PATH env var > config.json in CWD
```

For individual settings:
```
ENV variable > config.json key > default value
```

- `ServerSettings` checks ENV first, falls back to `env->get("server.*")` from config.json, then uses hardcoded defaults.
- `HttpClient` reads only ENV variables for timeouts (no config.json fallback).

## Config File Path

The config file path is resolved in this order:

1. **`--config <path>`** or **`-c <path>`** CLI argument (highest priority)
2. **`CONFIG_PATH`** environment variable
3. **`config.json`** in the current working directory (default)

Example:
```bash
# CLI argument
./my_service --config /etc/myapp/config.json

# Environment variable
CONFIG_PATH=/etc/myapp/config.json ./my_service

# Default: looks for config.json in CWD
./my_service
```

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

Circuit breaker uses a **prefix** to allow multiple independent circuits (e.g. `HTTP_BROKER_`, `HTTP_AUTH_`). Replace `<PREFIX>` below with your prefix.

| ENV Variable | Type | Default | Description |
|---|---|---|---|
| `<PREFIX>_CB_FAILURE_THRESHOLD` | int | `5` | Consecutive failures before opening circuit |
| `<PREFIX>_CB_RESET_TIMEOUT_MS` | int | `30000` | Time (ms) to wait before HALF_OPEN transition |
| `<PREFIX>_CB_HALF_OPEN_MAX_CALLS` | int | `3` | Successful calls in HALF_OPEN to close circuit |

Example:
```bash
# For HTTP broker calls
HTTP_BROKER_CB_FAILURE_THRESHOLD=5
HTTP_BROKER_CB_RESET_TIMEOUT_MS=30000
HTTP_BROKER_CB_HALF_OPEN_MAX_CALLS=3
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