# Core Utilities

Utility classes provided by microservice-core.

---

## UuidGenerator

Thread-safe ID generator. Produces 32-character hex string (128 bits).

```cpp
#include "UuidGenerator.hpp"
```

### Implementation

Uses `thread_local` Mersenne Twister — zero contention between threads.

```
Algorithm: 128-bit output = hi(64) + lo(64)
  hi = timestamp_ns XOR (random << 32)
  lo = counter XOR random
```

### Usage

```cpp
auto idGen = std::make_shared<UuidGenerator>();
std::string id = idGen->generate();  // "a1b2c3d4e5f67890..."
```

### Warning

Does NOT produce RFC 4122/RFC 9562 compliant UUIDs. No version nibble, variant bits, or hyphens. For RFC-compliant UUIDs, see SRV-44 (UUID v7 migration) in backlog.

---

## Timer

Measures elapsed time between `start()` and `stop()`.

```cpp
#include "Timer.hpp"
```

### Usage

```cpp
Timer t;
t.start();
// ... work ...
t.stop();
int64_t ms = t.elapsed(TimeUnit::Millis);
std::string s = t.show();  // "42ms"
```

### TimeUnit

```cpp
enum class TimeUnit { Nanos, Micros, Millis, Seconds };
t.elapsed(TimeUnit::Micros);  // microseconds
t.show(TimeUnit::Seconds);   // "3s"
```

### Behavior

- If `stop()` not called: `elapsed()`/`show()` return time since `start()`
- If never started: returns 0

---

## StringUtils

Static utility methods for string manipulation.

```cpp
#include "StringUtils.hpp"
```

### Methods

```cpp
std::string lower = StringUtils::toLower("Hello");       // "hello"
std::string lower = StringUtils::toLower("Already");      // "already"

std::vector<std::string> segs = StringUtils::splitPath("/api/users/123");
// {"api", "users", "123"}

std::string escaped = StringUtils::escapeJson("hello \"world\"");
// "hello \"world\""

std::string decoded = StringUtils::urlDecode("hello%20world");
// "hello world"
```

---

## ThreadSafeMap

Thread-safe key-value map with read-write locking.

```cpp
#include "ThreadSafeMap.hpp"
```

### Interface

```cpp
void put(const K& key, const V& value);
std::optional<V> get(const K& key) const;
bool remove(const K& key);
bool contains(const K& key) const;
size_t size() const;
void clear();
```

### Usage

```cpp
ThreadSafeMap<std::string, std::string> cache;
cache.put("key", "value");
auto val = cache.get("key");
```

---

## PathParamExtractor

Extracts path parameters by name from request.

```cpp
#include "PathParamExtractor.hpp"
```

### Usage

```cpp
// Pattern: /api/users/:id/orders/:orderId
// Request: /api/users/123/orders/456

PathParamExtractor extractor(req, "/api/users/:id/orders/:orderId");
auto userId = extractor.get("id");    // "123"
auto orderId = extractor.get("orderId");  // "456"
```

---

## See Also

- [Interfaces](interfaces.md) — IIdGenerator
- [Handlers](handlers.md) — ChainHandler, HealthHandler