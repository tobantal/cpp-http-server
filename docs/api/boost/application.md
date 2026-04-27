# Boost Application

Production-ready HTTP server on Boost.Beast/Asio.

---

## BoostBeastApplication

Main HTTP server class. Inherits from `BaseWebApplication`.

```cpp
#include "BoostBeastApplication.hpp"
```

### Construction

```cpp
// With custom logger
auto logger = std::make_shared<ConsoleLogger>();
auto app = std::make_shared<BoostBeastApplication>(logger);

// With default NullLogger
auto app = std::make_shared<BoostBeastApplication>();
```

### Lifecycle (Template Method from IWebApplication)

```cpp
int run(int argc, char* argv[]);
```

`run()` executes:
1. `loadEnvironment(argc, argv)` — parse ENV and config.json
2. `configureInjection()` — override in subclass
3. `start()` — begin accepting connections

### Methods

```cpp
void start() override;    // Start accepting connections
void stop() override;     // Stop accepting, close connections
void shutdown(timeoutMs); // Graceful shutdown with timeout

static std::string getVersion();  // Returns CPP_HTTP_SERVER_VERSION
```

### Configuration

Load from config.json:
```cpp
app->loadEnvironment(argc, argv);  // Looks for config.json in current directory
```

Or pass custom path:
```cpp
// Modify loadEnvironment to accept path, or set WORKING_DIR env
```

---

## BaseWebApplication

Boost-independent base class in microservice-core.

```cpp
#include "BaseWebApplication.hpp"
```

### Handler Registration

```cpp
// Variadic — creates ChainHandler internally
registerEndpoint("GET", "/api/users", handler1, handler2, handler3);

// Register single handler directly
registerHandler("GET", "/api/users", handler);
```

### Request Handling

`handleRequest(IRequest&, IResponse&)` — routes to registered handlers, handles errors.

### ServerState Enum

```cpp
enum class ServerState : uint8_t { NotStarted, Running, Stopped };
std::atomic<ServerState> state_{ServerState::NotStarted};
```

---

## Connection Management

### Limits

- `maxRequestBodySize_` — max request body size (default from config)
- `maxConnections_` — max concurrent connections (0 = unlimited)
- `maxRequestsPerConnection_` — requests per keep-alive connection (default: 100)

### Timeouts

- `readTimeout_` — read timeout in milliseconds
- `writeTimeout_` — write timeout in milliseconds

### Active Connections

`activeConnections_` — atomic counter for current connection count.

---

## See Also

- [Core Interfaces](../core/interfaces.md) — IWebApplication, IRequest, IResponse
- [Core Handlers](../core/handlers.md) — ChainHandler, HealthHandler
- [Configuration](../../configuration.md) — ENV and config.json reference