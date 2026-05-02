# Core Interfaces

Core interfaces provide pure abstractions with zero external dependencies.

---

## IRequest

HTTP request abstraction. All path, query, headers, body access.

```cpp
#include "IRequest.hpp"
```

### Path Operations

```cpp
std::string path = req->getPath();           // URL-decoded, no query string
std::vector<std::string> segs = req->getPathSegments();
std::optional<std::string> param = req->getPathParam(0);  // Wildcard index
```

### Query Parameters

```cpp
auto params = req->getQueryParams();         // All params
auto name = req->getQueryParam("id");        // Single param
req->setQueryParam("page", "2");
```

### Headers

```cpp
auto headers = req->getHeaders();
auto auth = req->getHeader("Authorization");
req->setHeader("X-Request-ID", "12345");
```

### Body

```cpp
std::string body = req->getBody();
req->setBody(R"({"name":"test"})");
```

### Convenience

```cpp
auto token = req->getBearerToken();         // Extract from Authorization header
bool json = req->isJson();                   // Content-Type check
std::string ct = req->getContentType();

std::string ip = req->getIp();
int port = req->getPort();
```

### Attributes (for passing data between handlers)

```cpp
req->setAttribute("userId", "12345");
auto userId = req->getAttribute("userId");
```

### Trace ID

```cpp
std::string traceId = req->getTraceId();     // From header or generated
req->setTraceId("custom-trace-id");
```

---

## IResponse

HTTP response abstraction. Status, body, headers.

```cpp
#include "IResponse.hpp"
```

### Status

```cpp
res->setStatus(200);
res->setStatus(HttpStatus::Ok);
int code = res->getStatus();
```

### Body

```cpp
res->setBody(R"({"message":"success"})");
std::string body = res->getBody();
```

### Headers

```cpp
res->setHeader("X-Trace-ID", "abc123");
auto headers = res->getHeaders();
auto contentType = res->getHeader("Content-Type");
```

### Cookies

```cpp
res->setCookie("sessionId", "abc123", "/", true, false, 3600);
```

### Convenience

```cpp
res->setResult(200, "application/json", R"({"ok":true})");
res->setResult(HttpStatus::Created, "application/json", R"({"id":1})");
```

### Trace ID

```cpp
res->setTraceId("abc123");
```

---

## IHttpHandler

Interface for HTTP request handlers.

```cpp
#include "IHttpHandler.hpp"
```

### Interface

```cpp
class IHttpHandler {
public:
    virtual ~IHttpHandler() = default;

    virtual void handle(
        std::shared_ptr<IRequest> req,
        std::shared_ptr<IResponse> res) = 0;

    virtual std::string name() const = 0;
};
```

### Usage

Implement `handle(req, res)` — set status, body, headers. Use `name()` for logging.

---

## IWebApplication

Template Method pattern for application lifecycle.

```cpp
#include "IWebApplication.hpp"
```

### Lifecycle (run method)

```cpp
int run(int argc, char* argv[]);
```

`run()` executes: `loadEnvironment()` → `configureInjection()` → `start()`.

### Subclass Responsibility

```cpp
class MyApp : public BoostBeastApplication {
    void configureInjection() override {
        // Register handlers, configure DI
    }
};
```

### registerEndpoint

```cpp
// Variadic — accepts multiple handlers for middleware chain
registerEndpoint("GET", "/api/users", handler1, handler2, handler3);
```

---

## IShutdown

Interface for graceful shutdown.

```cpp
#include "IShutdown.hpp"
```

### Methods

```cpp
void shutdown(std::chrono::milliseconds timeoutMs = std::chrono::milliseconds(5000));
void stop();
```

### ShutdownManager

LIFO shutdown order for multiple components:

```cpp
auto shutdownMgr = std::make_shared<ShutdownManager>(logger);
shutdownMgr->registerComponent(app);
shutdownMgr->shutdownAll();
```

---

## IHttpErrorHandler

Error handling interface.

```cpp
#include "IHttpErrorHandler.hpp"
```

```cpp
class IHttpErrorHandler {
public:
    virtual void handleError(
        std::shared_ptr<IResponse> res,
        const HttpError& error) = 0;
};
```

Default implementation: `HttpErrorSender` — JSON error responses.

---

## IHttpClient

HTTP client for outgoing requests.

```cpp
#include "IHttpClient.hpp"
```

```cpp
class IHttpClient {
public:
    virtual ~IHttpClient() = default;
    virtual HttpClientResult send(const IRequest& request, IResponse& response) = 0;
};
```

Decorators: `RetryingHttpClient` (retry logic), `CircuitBreakingHttpClient` (circuit breaker).

---

## ICircuitBreaker

Circuit breaker interface with CLOSED/OPEN/HALF_OPEN states.

```cpp
#include "ICircuitBreaker.hpp"
```

```cpp
enum class CircuitState : uint8_t { Closed, Open, HalfOpen };

class ICircuitBreaker {
public:
    virtual ~ICircuitBreaker() = default;
    virtual bool allowsCall() = 0;
    virtual void recordSuccess() = 0;
    virtual void recordFailure() = 0;
    virtual void recordFailure(HttpClientError error) = 0;
    virtual CircuitState state() const = 0;
};
```

Implementation: `CircuitBreaker` — thread-safe state machine with configurable thresholds.

---

## ICircuitBreakerSettings

Circuit breaker configuration interface.

```cpp
#include "ICircuitBreakerSettings.hpp"
```

```cpp
class ICircuitBreakerSettings {
public:
    virtual int getFailureThreshold() const = 0;
    virtual std::chrono::milliseconds getResetTimeout() const = 0;
    virtual int getHalfOpenMaxCalls() const = 0;
};
```

Implementation: `CircuitBreakerSettings` — reads from `<PREFIX>_CB_*` environment variables.

---

## IEnvironment

Configuration access interface.

```cpp
#include "IEnvironment.hpp"
```

```cpp
virtual std::optional<std::string> get(const std::string& key) const = 0;
virtual std::string getOrDefault(const std::string& key, const std::string& defaultValue) const = 0;
```

Implementation: `Environment` — ENV → config.json → default.

---

## IIdGenerator

Interface for ID generation (DIP for testability).

```cpp
#include "IIdGenerator.hpp"
```

```cpp
virtual std::string generate() = 0;
```

Implementation: `UuidGenerator` — thread-safe UUID v4 generation.

---

## See Also

- [Handlers](handlers.md) — ChainHandler, HealthHandler, MetricsHandler
- [Utilities](utilities.md) — UuidGenerator, Timer, StringUtils
- [Boost Application](../boost/application.md) — BoostBeastApplication, BaseWebApplication