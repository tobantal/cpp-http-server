# Testing Guidelines

Testing standards for cpp-http-server using Google Test.

---

## Framework and Location

- **Google Test (GTest)** — primary test framework
- microservice-core tests → `microservice-core/tests/`
- microservice-boost tests → `microservice-boost/tests/`
- CI: GitHub Actions (cmake build + ctest + coverage)

---

## Naming Convention

```
TestSuiteName.TestScenario_ExpectedResult
```

Examples:
```
RouteMatcherTest.WildcardMiddle_MatchesPath
BeastRequestAdapter.GetPathParam_ReturnsValue
BoostBeastApplication.HandleRequest_Returns404ForUnknownRoute
```

---

## What to Test

### Core Module

- `IRequest`/`IResponse` — via `SimpleRequest`/`SimpleResponse` test doubles
- `RouteMatcher` — exact match, wildcards, edge cases (trailing slash, empty path, multiple wildcards)
- `ChainHandler` — short-circuit, full chain, empty chain, single handler
- `Environment` — type access, missing keys, defaults

### Boost Module

- `BeastRequestAdapter` — all IRequest methods via real Beast request
- `BeastResponseAdapter` — all IResponse methods
- `BoostBeastApplication` — lifecycle (start/stop), routing, path params, method not allowed
- `HttpClient` — connect, timeout (with mock server), error handling
- `ServerSettings` — ENV priority, config.json fallback, defaults

### Integration

- Start real server → HTTP request → verify response (as in HttpClientTest)

---

## Test Doubles

- `SimpleRequest`/`SimpleResponse` — main test doubles for IRequest/IResponse
- For HttpClient testing: start real server on random port (as in HttpClientTest)
- For BoostBeastApplication testing: start server with test handlers

---

## Before Commit — Mandatory

1. **Unit tests** — `cd build && ctest --verbose` — all must pass
2. **Coverage** — verify new lines are covered
3. **Backward compatibility** — consumer project (trading-platform) must build without changes

---

## Forbidden

- Skipping failing tests
- Using `sleep()` in tests (use mocks for time, or short timeouts)
- Hardcoded ports in tests (use port 0 — OS assigns free port, or random from range)

---

## Known Test Gaps

- No concurrency/thread-safety tests (ThreadSafeMap and handlers_ map)
- No timeout-scenario tests for HttpClient
- No request body size limit tests
- No malformed HTTP request tests
- No stress/load tests