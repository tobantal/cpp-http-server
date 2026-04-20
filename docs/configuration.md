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

## HTTP Client Settings (outgoing requests)

| ENV Variable | Type | Default | Description |
|---|---|---|---|
| `HTTP_CLIENT_CONNECT_TIMEOUT_MS` | int | `5000` | Connect timeout in ms (5s) |
| `HTTP_CLIENT_READ_TIMEOUT_MS` | int | `30000` | Read timeout in ms (30s) |
| `HTTP_CLIENT_WRITE_TIMEOUT_MS` | int | `30000` | Write timeout in ms (30s) |

## config.json Example

```json
{
  "server": {
    "host": "0.0.0.0",
    "port": 8080,
    "maxRequestBodySize": 1048576,
    "readTimeoutMs": 30000,
    "writeTimeoutMs": 30000,
    "maxConnections": 0
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