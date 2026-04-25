# cpp-http-server

A modern C++17 HTTP server library with clean hexagonal architecture (ports-and-adapters).

**Author:** Anton Tobolkin

---

## Overview

Two-module structure:

| Module | Description |
|--------|-------------|
| `microservice-core` | Interfaces and utilities — zero dependencies, header-only + RouteMatcher.cpp |
| `microservice-boost` | Production-ready HTTP server on Boost.Beast/Asio |

### Requirements

- **C++17** or higher
- **CMake 3.14+**
- **Boost 1.70+** (Asio, Beast, System)
- **nlohmann/json** 3.9+ (for config.json support in microservice-boost)

---

## Installation

### CMake FetchContent

```cmake
cmake_minimum_required(VERSION 3.14)
project(my_service)

include(FetchContent)

FetchContent_Declare(
    cpp-http-server
    GIT_REPOSITORY https://github.com/tobantal/cpp-http-server.git
    GIT_TAG v0.4.0
)

# If your project already pulls Boost/nlohmann_json:
# set(CPP_HTTP_SERVER_FETCH_DEPS OFF)

FetchContent_MakeAvailable(cpp-http-server)

add_executable(my_app src/main.cpp)
target_link_libraries(my_app microservice-boost)
```

### Build and Tests

```bash
git clone https://github.com/tobantal/cpp-http-server.git
cd cpp-http-server
cmake -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build
cd build && ctest --verbose
```

---

## Quick Start

### Application

```cpp
#include "BoostBeastApplication.hpp"
#include "ChainHandler.hpp"
#include "HealthHandler.hpp"

class MyWebApp : public BoostBeastApplication {
public:
    void configureInjection() override {
        registerEndpoint("GET", "/health", std::make_shared<HealthHandler>());
        registerEndpoint("GET", "/api/users", std::make_shared<GetUsersHandler>());
    }
};
```

### main.cpp — 3 lines (signal handling built-in)

```cpp
#include "MyWebApp.hpp"

int main(int argc, char* argv[]) {
    MyWebApp app;
    return app.run(argc, argv);
}
```

Signal handling (SIGINT/SIGTERM) is built into `IWebApplication::run()`. Try/catch is also inside.

### Configuration

See [docs/configuration.md](docs/configuration.md) for full settings reference and [.env.example](.env.example) for ENV variables.

---

## Key Features

### Graceful Shutdown

```cpp
auto app = std::make_shared<MyWebApp>();
auto shutdownMgr = std::make_shared<ShutdownManager>(logger);
shutdownMgr->registerComponent(app);  // app implements IShutdown
// SIGINT/SIGTERM → ShutdownManager::shutdownAll() in LIFO order
```

### Metrics (Prometheus)

```cpp
auto metrics = std::make_shared<MetricsCollector>();
auto observer = std::make_shared<MetricsObserverHandler>(chainHandler, metrics, "my-service");
auto metricsHandler = std::make_shared<MetricsHandler>(metrics);
registerEndpoint("GET", "/metrics", metricsHandler);
```

### Error Handling (IHttpErrorHandler)

```cpp
class XmlErrorHandler : public IHttpErrorHandler {
    void handleError(IResponse& res, const HttpError& e) override {
        res.setResult(e.statusCode(), "application/xml",
            "<error>" + e.message() + "</error>");
    }
};
```

### Keep-Alive

Multiple requests on single TCP connection (up to `SERVER_MAX_REQUESTS_PER_CONNECTION=100`).

### Connection Limits

- `SERVER_MAX_CONNECTIONS=0` — unlimited, 503 on exceed
- `SERVER_MAX_REQUEST_BODY_SIZE=1048576` — 413 on exceed
- Timeouts: `SERVER_READ_TIMEOUT_MS`, `SERVER_WRITE_TIMEOUT_MS`

---

## SOLID Principles

- **S** — single responsibility (ChainHandler = middleware, MetricsObserverHandler = metrics, etc.)
- **O** — extension via interfaces: `IHttpHandler`, `IMetricsCollector`, `IHttpErrorHandler`, `IShutdown`, `IIdGenerator`
- **L** — implementations interchangeable: `UuidGenerator` ↔ deterministic mock in tests
- **I** — `INameable` separated from `IHttpHandler` and `IShutdown`
- **D** — dependencies on abstractions: `ChainHandler` depends on `IHttpErrorHandler`, not `HttpErrorSender`

---

## Documentation

| Document | Description |
|----------|-------------|
| [docs/getting-started.md](docs/getting-started.md) | Step-by-step guide for new users |
| [docs/architecture.md](docs/architecture.md) | Architecture, patterns, decisions |
| [docs/api/](docs/api/) | API Reference — detailed interface documentation |
| [docs/configuration.md](docs/configuration.md) | ENV + config.json reference |
| [docs/development/](docs/development/) | Development guidelines |

---

## License

MIT License. See [LICENSE](LICENSE) for details.