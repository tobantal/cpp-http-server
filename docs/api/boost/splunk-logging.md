# Splunk Logging

Integration with Splunk Enterprise/Cloud via HTTP Event Collector (HEC).

---

## Quick Start

### 1. Start Splunk Container

```bash
docker run -d --name splunk \
  -p 8000:8000 -p 8088:8088 \
  -e SPLUNK_START_ARGS=--accept-license \
  -e SPLUNK_PASSWORD=YourPassword123 \
  splunk/splunk:latest
```

- **Splunk Web UI:** http://localhost:8000 (user: admin, password: YourPassword123)
- **HEC Endpoint:** http://localhost:8088/services/collector

### 2. Configure HTTP Event Collector

1. Open Splunk Web UI: http://localhost:8000
2. Go to **Settings → Data inputs → HTTP Event Collector**
3. Click **Enable global token** (top of page)
4. Click **New Token** to create a new HEC token
5. Name: `app-logs`, click Next
6. App context: select your app or `main`
7. Output group: default
8. Review and submit
9. **Copy the token value** — you'll need it for `APP_SPLUNK_TOKEN`

### 3. Configure Application

Set environment variables:

```bash
# Splunk Connection
APP_SPLUNK_URL=http://localhost:8088/services/collector
APP_SPLUNK_TOKEN=your-hec-token-here
APP_SPLUNK_INDEX=main
APP_SPLUNK_SOURCETYPE=_json

# Buffering (default: 100 entries, 5 sec)
APP_SPLUNK_BUFFER_SIZE=100
APP_SPLUNK_FLUSH_INTERVAL_SEC=5
```

### 4. Use in Application

```cpp
#include "adapters/secondary/SplunkLogger.hpp"
#include "settings/SplunkLogSettings.hpp"

auto httpClient = std::make_shared<HttpClient>();
auto settings = std::make_shared<SplunkLogSettings>("APP");
auto fallbackLogger = std::make_shared<ConsoleLogger>();

auto logger = std::make_shared<SplunkLogger>(
    httpClient,
    settings,
    fallbackLogger  // called if Splunk is unavailable
);

// Register for graceful shutdown (flushes pending logs)
shutdownManager->registerComponent(logger);

logger->log(LogLevel::Info, "App", "User logged in");
logger->log(LogLevel::Error, "Auth", "Invalid credentials");
```

---

## Architecture

```
ILogger (port) ──────────────────────► ILogger (port)
    │                                       │
IShutdown                               IShutdown
    │                                       │
    └───── SplunkLogger (standalone) ◄──────┘
              │
              └──► ISplunkLogSettings
                        │
                        └──► SplunkLogSettings
```

### Classes

| Class | Purpose |
|-------|---------|
| `ISplunkLogSettings` | Interface with 6 methods: url, token, index, sourcetype, bufferSize, flushInterval |
| `SplunkLogSettings` | ENV reader: `*_SPLUNK_URL`, `*_SPLUNK_TOKEN`, etc. |
| `SplunkLogger` | Async logger (ILogger + IShutdown), sends logs to Splunk HEC |

---

## Environment Variables

All settings use prefix (e.g., `APP_`, `SERVICE_`).

| Variable | Default | Description |
|----------|---------|-------------|
| `<PREFIX>_SPLUNK_URL` | http://localhost:8088/services/collector | HEC endpoint |
| `<PREFIX>_SPLUNK_TOKEN` | (empty) | HEC token (required) |
| `<PREFIX>_SPLUNK_INDEX` | main | Splunk index |
| `<PREFIX>_SPLUNK_SOURCETYPE` | _json | Splunk sourcetype |
| `<PREFIX>_SPLUNK_BUFFER_SIZE` | 100 | Max entries before flush |
| `<PREFIX>_SPLUNK_FLUSH_INTERVAL_SEC` | 5 | Flush interval in seconds |

---

## Splunk Search Examples

### Find all logs from your application

```spl
index=main sourcetype=_json
```

### Filter by category

```spl
index=main sourcetype=_json category="App"
```

### Filter by log level

```spl
index=main sourcetype=_json level="Error"
```

### Search for errors in time range

```spl
index=main sourcetype=_json level="Error" earliest=-1h
```

### Full JSON event structure

```json
{
  "event": {
    "level": "ERROR",
    "category": "Auth",
    "message": "Invalid credentials",
    "timestamp": 1714339200000
  },
  "index": "main",
  "sourcetype": "_json"
}
```

---

## Troubleshooting

### Logs not appearing in Splunk

1. Verify HEC token is valid
2. Check Splunk logs: Settings → Data inputs → HTTP Event Collector → token → View knowledge objects
3. Ensure `APP_SPLUNK_URL` points to correct endpoint
4. Check network connectivity to Splunk

### Using fallback logger

If Splunk is unavailable, logs are written to fallback logger (ConsoleLogger by default). Check fallback logs for errors like:

- `URL not configured` — `APP_SPLUNK_URL` not set
- `Failed to send logs` — HTTP request failed

### Graceful shutdown

SplunkLogger implements `IShutdown`. Register it with ShutdownManager to ensure pending logs are flushed before exit:

```cpp
shutdownManager->registerComponent(logger);
// On SIGINT/SIGTERM → shutdownManager->shutdownAll()
```

---

## See Also

- [Boost Adapters](adapters.md) — HttpClient
- [Core Interfaces](../../core/interfaces.md) — ILogger, IShutdown
- [Configuration](../../configuration.md) — ENV variables