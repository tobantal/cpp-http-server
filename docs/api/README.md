# API Reference

Detailed documentation for cpp-http-server interfaces.

---

## Core Module

### Interfaces

- [IRequest, IResponse](core/interfaces.md) — HTTP request/response abstractions
- [IHttpHandler](core/interfaces.md) — handler interface
- [IWebApplication](core/interfaces.md) — application lifecycle
- [IShutdown](core/interfaces.md) — graceful shutdown
- [IHttpErrorHandler](core/interfaces.md) — error handling
- [IHttpClient](core/interfaces.md) — HTTP client interface
- [IEnvironment](core/interfaces.md) — configuration access
- [IIdGenerator](core/interfaces.md) — ID generation

### Handlers

- [ChainHandler](core/handlers.md) — middleware chain
- [HealthHandler](core/handlers.md) — health endpoint
- [MetricsHandler](core/handlers.md) — Prometheus metrics
- [MetricsObserverHandler](core/handlers.md) — metrics decorator

### Utilities

- [UuidGenerator](core/utilities.md) — thread-safe ID generation
- [Timer](core/utilities.md) — elapsed time measurement
- [StringUtils](core/utilities.md) — string manipulation
- [ThreadSafeMap](core/utilities.md) — concurrent map
- [PathParamExtractor](core/utilities.md) — named path parameters

---

## Boost Module

### Application

- [BoostBeastApplication](boost/application.md) — HTTP server
- [BaseWebApplication](boost/application.md) — boost-independent base

### Adapters

- [BeastRequestAdapter](boost/adapters.md) — Beast → IRequest
- [BeastResponseAdapter](boost/adapters.md) — IResponse → Beast
- [HttpClient](boost/adapters.md) — HTTP client for outgoing requests