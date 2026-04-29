# Security Guidelines

Mandatory security rules for HTTP server. Violation = critical bug. Review at every PR.

---

## HTTP Server Security

### Request Validation

- **Request body size limit** — mandatory limit (default 1MB, configurable). Without limit = DoS via huge body
- **Request timeout** — mandatory read timeout (default 30s). Without timeout = slowloris DoS
- **URL decoding** — decode path and query params, check for path traversal (`..`)
- **Header size limit** — limit on header size (default 8KB)

### Thread-Safety

- **`handlers_` map** — access from detached threads → synchronization needed or immutability after start()
- **`state_`** — `std::atomic<ServerState>`, states: NotStarted, Running, Stopped. Access from main thread + signal handler. `registerHandler()` allowed only in NotStarted. `stop()` uses `compare_exchange(Running, Stopped)`
- **Request/Response objects** — created per-session, not shared between threads. OK
- **Detached threads** — lifetime hazard: `this` can be destroyed while thread runs. Solution: thread pool + join on stop()

### Resource Limits

- **Max connections** — limit concurrent connections (no limit = DoS via thread exhaustion)
- **Thread pool** — replace detached threads with fixed-size pool
- **Keep-alive** — limit requests per connection (default 100)

### Response Security

- Don't include server version/OS in response headers
- Don't include stack traces in 500 errors (only `{"error": "Internal server error"}`)
- Set `X-Content-Type-Options: nosniff`
- Set `X-Frame-Options: DENY`

---

## HTTP Client Security

- **Timeouts** — connect, read, write (no timeouts = cascade hang)
- **SSL/TLS** — for HTTPS endpoints (verify cert, don't skip)
- **No plaintext credentials** in URL or headers, except explicitly configured
- **Redirect limit** — don't follow redirects infinitely

---

## Configuration Security

- **Config path** — don't hardcode `config.json`, configure via ENV or CLI
- **Secrets** — don't store in config.json, only in ENV / K8s Secrets
- **Port** — don't bind to 0.0.0.0 without explicit config (default: localhost)