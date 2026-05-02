# Boost Adapters

Adapters converting between Boost.Beast types and library interfaces.

---

## BeastRequestAdapter

Converts Boost.Beast HTTP request to `IRequest` interface.

```cpp
#include "BeastRequestAdapter.hpp"
```

### Construction

```cpp
BeastRequestAdapter adapter(
    beastRequest,   // Boost.Beast request
    clientIp,       // string
    port = 80,      // int
    idGenerator     // std::shared_ptr<IIdGenerator>
);
```

### Key Implementation Details

- `getPath()` — URL-decodes path, strips query string
- `getQueryParams()` — cached after first call
- `getHeader()` — case-insensitive lookup
- `getTraceId()` — from `X-Trace-ID` header or generates via `idGenerator_`

---

## BeastResponseAdapter

Converts `IResponse` interface to Boost.Beast HTTP response.

```cpp
#include "BeastResponseAdapter.hpp"
```

### Usage

Created internally by `BoostBeastApplication` when handling requests.

---

## HttpClient

HTTP client for outgoing requests using Boost.Asio/Beast.

```cpp
#include "HttpClient.hpp"
```

### Construction

```cpp
HttpClient logger(std::make_shared<ConsoleLogger>());
HttpClient();  // Uses NullLogger by default
```

### Send Request

```cpp
HttpClientResult result = client.send(request, response);

if (result.isSuccess()) {
    // response is populated
} else {
    // result.error() contains error info
}
```

### Timeouts

From ENV or defaults:
- `HTTP_CLIENT_CONNECT_TIMEOUT_MS` — default 5000ms
- `HTTP_CLIENT_READ_TIMEOUT_MS` — default 30000ms
- `HTTP_CLIENT_WRITE_TIMEOUT_MS` — default 30000ms

---

## RetryingHttpClient

Decorator that adds retry logic to `IHttpClient`.

```cpp
#include "RetryingHttpClient.hpp"
```

### Construction

```cpp
auto retrying = std::make_shared<RetryingHttpClient>(httpClient, retryExecutor);
```

Uses `IHttpRetryExecutor` for retry policy with exponential backoff.

---

## CircuitBreakingHttpClient

Decorator that adds circuit breaker protection to `IHttpClient`.

```cpp
#include "CircuitBreakingHttpClient.hpp"
```

### Construction

```cpp
auto cbProtected = std::make_shared<CircuitBreakingHttpClient>(httpClient, circuitBreaker, logger);
```

### Behavior

- **CLOSED** — requests pass through normally
- **OPEN** — rejects immediately with `HttpClientError::ConnectionRefused`
- **HALF_OPEN** — allows limited requests; success closes circuit, failure reopens

---

## See Also

- [Boost Application](application.md) — BoostBeastApplication
- [Core Interfaces](../../core/interfaces.md) — IRequest, IResponse, IHttpClient