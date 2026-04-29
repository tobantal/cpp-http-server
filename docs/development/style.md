# C++ Style Guide

Coding standards for cpp-http-server.

---

## C++ Standard

- **C++17** — do not change without agreement
- Compilers: GCC 12+, Clang 15+
- CMake 3.14+

---

## Project Structure

### microservice-core/

Interfaces and utilities — zero dependencies.

```
include/
  domain/           # IRequest, IResponse, INameable, HttpError
  error/            # BadRequestError, NotFoundError, etc.
  ports/input/      # IHttpHandler, IWebApplication
  ports/output/     # ILogger, IShutdown, IHttpClient
  application/      # ChainHandler, ShutdownManager
  handler/          # HealthHandler, MetricsHandler
  metrics/          # MetricsCollector, IMetricsCollector
  util/             # UuidGenerator, Timer, StringUtils
  settings/         # IServerSettings, Environment
  adapters/         # SimpleRequest, SimpleResponse, NullLogger
src/
  RouteMatcher.cpp  # Only .cpp in core
```

### microservice-boost/

Boost.Beast implementation.

```
include/
  BoostBeastApplication.hpp
  adapters/primary/   # BeastRequestAdapter, BeastResponseAdapter
  adapters/secondary/  # HttpClient, ConnectionPool
  handler/            # JsonValidator
  settings/           # ServerSettings, DbSettings
src/
  BoostBeastApplication.cpp
  HttpClient.cpp
```

---

## Dependency Rule

- microservice-core has **no dependencies** on Boost, nlohmann/json, or any third-party library
- microservice-boost depends on microservice-core + Boost + Boost.DI + nlohmann/json

---

## File Structure

### Class Organization

1 class = 1 file (exception: tightly coupled small classes)

```cpp
// Header order:
#pragma once
// 1. Corresponding .hpp (for .cpp files)
// 2. < > system and third-party (Boost, nlohmann)
// 3. " " project headers
// 4. Empty line between groups
```

### Header Guards

```cpp
#pragma once
// or
#ifndef MICROSERVICE_CORE_IREQUEST_HPP
#define MICROSERVICE_CORE_IREQUEST_HPP
// ...
#endif
```

### Section Order

`public` → `protected` → `private`
Methods: constructor → destructor → public → protected → private

---

## Naming

| Element | Style | Example |
|---------|-------|---------|
| Class/Struct | PascalCase | `BoostBeastApplication` |
| Interface | `I` prefix + PascalCase | `IRequest`, `IHttpHandler` |
| Function/Method | camelCase | `getPathParam()` |
| Variable | camelCase | `queryParams` |
| Private field | camelCase + `_` suffix | `handlers_`, `ioContext_` |
| Constant | kPascalCase | `kDefaultPort` |
| Enum value | PascalCase | `HttpMethod::Get` |
| Namespace | snake_case | `microservice_core` |
| File | PascalCase | `BeastRequestAdapter.hpp` |
| Macro | UPPER_SNAKE | `MICROSERVICE_CORE_FILE_HPP` |

---

## Memory Management

- `std::shared_ptr` — for handlers (shared ownership through DI)
- `std::unique_ptr` — for sockets, io_context, acceptor
- No raw `new`/`delete`
- RAII — all resources (sockets, connections) in RAII objects

---

## Const Correctness

- Reference parameters: `const std::string&` if not modified
- Methods: `const` if they don't change state
- Iterators: `cbegin()`/`cend()` if not modifying

---

## Struct Field Ordering

Order fields by decreasing size (8 bytes → 4 → 2 → 1) to minimize padding:

```cpp
// CORRECT: large fields first
struct Order {
    std::string id;              // 32 bytes
    std::string accountId;       // 32 bytes
    int64_t quantity = 0;        // 8 bytes
    int64_t executedQty = 0;     // 8 bytes
    Timestamp createdAt;         // 8 bytes
    OrderDirection direction;    // 1 byte (enum class : uint8_t)
};

// WRONG: enum between string and int64_t causes padding
struct OrderBad {
    std::string id;
    OrderDirection direction;    // 1 byte → 7 bytes padding
    int64_t quantity = 0;        // padding before this
    ...
};
```

### Enum Class

Always use `uint8_t` as underlying type:

```cpp
enum class ServerState : uint8_t { NotStarted, Running, Stopped };
enum class LogLevel : uint8_t { Debug, Info, Warn, Error };
```

This saves 3 bytes per enum (4 → 1).

---

## Header-Only vs hpp/cpp

- **Interfaces** (pure `virtual` = 0, no implementation) — header-only, size limit not enforced (IRequest.hpp at 296 lines is fine, it's a pure interface + docs)
- **Classes with implementation** — always in `.cpp`, `.hpp` contains declaration only
- **Templates** — must stay in `.hpp`, use `#include "*.impl.hpp"` or inline

---

## Doxygen Documentation

All public headers in `include/` must have Doxygen comments.

### File Comment

```cpp
/**
 * @file ChainHandler.hpp
 * @brief Middleware chain handler
 * @author Anton Tobolkin
 */
```

### Class Comment

```cpp
/**
 * @class ChainHandler
 * @brief Middleware chain — executes handlers sequentially
 *
 * Executes each handler in order of addition. On HttpError,
 * delegates to IHttpErrorHandler.
 */
```

### Method Comment

```cpp
/**
 * @brief Register HTTP handler for method and path pattern
 * @param method HTTP method (GET, POST, etc.)
 * @param pattern URL pattern, supports * wildcard
 * @param handler Handler instance
 */
void registerHandler(const std::string& method,
                    const std::string& pattern,
                    std::shared_ptr<IHttpHandler> handler);
```

### Private Method

One-line brief:

```cpp
/** @brief Find handler by method and path */
std::optional<HandlerMatch> findHandler(const std::string& method, const std::string& path);
```

### What NOT to Do

- Don't duplicate method name in `@brief`: `@brief handleRequest` is redundant
- Don't document `override` methods unless behavior differs from base class
- Don't add `@author` to every method — only to file
- Don't use `///` — use `/** */` Qt-style

---

## Pre-Commit Checks

Before each commit, run clang-tidy:

```bash
clang-tidy -p build --warnings-as-errors='*' <changed-files>
```

Fix issues before committing. Config: `.clang-tidy`.

Known suppressions: `pro-type-member-init`, `special-member-functions`, `use-nodiscard`, `enum-size`, `convert-member-functions-to-static`, `implicit-bool-conversion`, `named-parameter`, `easily-swappable-parameters`.