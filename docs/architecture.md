# Architecture

## Two-Module Structure

```
┌────────────────────────────────────────────────────────────┐
│                  microservice-core                         │
│  Interfaces and utilities — header-only + RouteMatcher.cpp │
│  Zero dependencies                                         │
│  IRequest, IResponse, IHttpHandler, IWebApplication       │
│  BaseWebApplication, ChainHandler, HealthHandler            │
│  IHttpClient, IEnvironment, Environment, RouteMatcher       │
│  RouteTrie, SimpleRequest, SimpleResponse, MetricsCollector │
├────────────────────────────────────────────────────────────┤
│                  microservice-boost                        │
│  Production implementation on Boost.Beast/Asio             │
│  Dependencies: Boost, Boost.DI, nlohmann/json             │
│  BoostBeastApplication, BeastRequestAdapter                 │
│  BeastResponseAdapter, HttpClient                          │
│  ServerSettings, ConnectionPool                           │
└────────────────────────────────────────────────────────────┘
```

**Dependency Rule:** microservice-boost depends on microservice-core. microservice-core does NOT depend on boost.

---

## Key Design Decisions

### BaseWebApplication — Boost-Independent Base

`BaseWebApplication` in core provides handler registration, request routing (exact + wildcard), and error handling. Boost-specific transport logic (io_context, acceptor, sessions) lives in `BoostBeastApplication`, which inherits from `BaseWebApplication`.

### Template Method (IWebApplication)

`run()` defines lifecycle: `loadEnvironment()` → `configureInjection()` → `start()`

### Adapter Pattern

`BeastRequestAdapter` and `BeastResponseAdapter` convert Boost.Beast objects to pure interfaces `IRequest`/`IResponse`.

### Chain of Responsibility (ChainHandler)

Middleware chain: handlers execute sequentially, status code != 0 breaks the chain.

### Status Code as Chain Signal

- `status == 0` = "continue chain"
- `status != 0` = "stop chain"

### Route Matching

Trie-based routing via `RouteTrie` with O(k) lookup (k = number of path segments):

- **Static segments**: `/api/users` matches `/api/users`
- **Named parameters**: `/users/:id` matches `/users/42`, extracted as `pathParams["id"] = "42"`
- **Wildcards**: `/files/*` matches `/files/readme.txt`
- **Priority**: static > `:param` > `*` (standard Express.js / gorilla/mux behavior)

`BaseWebApplication` registers routes into `RouteTrie::insert()` and resolves via `RouteTrie::lookup()`.
Path parameters are extracted during traversal and injected into `IRequest::setPathParams()`.

### Path Parameters

Two access methods on `IRequest`:
- `getPathParam(size_t index)` — by wildcard/param position (backward compat)
- `getPathParam(const std::string& name)` — by parameter name (`:paramName`)

---

## Thread Model

- Accept loop on main thread (blocking)
- Thread pool for connections (v0.4.0)
- Synchronous I/O in handleSession

---

## Quality Checklist

When making changes, verify:

1. **Separation core/boost** — core does not depend on boost
2. **Backward compatibility** — consumer project still builds
3. **Thread-safety** — concurrent access to shared state is safe
4. **Performance** — no O(n) bottlenecks
5. **Error handling** — all error paths lead to correct client response

---

## For More

- [API Reference](api/) — detailed interface documentation
- [Configuration](configuration.md) — ENV and config.json reference
- [Development Guidelines](development/) — coding standards