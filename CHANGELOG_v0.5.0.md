# Changelog v0.5.0 (WIP)

All changes targeting the v0.5.0 release. Collected here during development.

---

## SRV-11: Named path parameters (`:param` syntax)

**Task:** #136 | **Commit:** `b10fd94`

### Changes

- **RouteMatcher** — `isWildcard()`, `isNamedParam()`, `paramName()`: recognizes `:paramName` segments alongside `*`
- **PathParamExtractor** — `getByName()`: extract path parameters by name from `:param` patterns
- **IRequest** — `getPathParam(const std::string& name)`: new overload for named parameter access
- **SimpleRequest** — implements `getPathParam(string)` via `PathParamExtractor::getByName()`
- **BeastRequestAdapter** — implements `getPathParam(string)` via `PathParamExtractor::getByName()`
- **BaseWebApplication** — `hasParameters()`: fixes bug where `pattern.find('*')` missed `:param` patterns
- **Priority**: static > `:param` > `*` in `findHandler()` (two-pass scan)

### Backward Compatibility

- `getPathParam(size_t)` unchanged
- `*` patterns work as before
- `:param` is additive, not a replacement

### Tests

19 new tests: RouteMatcher (8), SimpleRequest (5), BaseWebApplication (5), BeastRequestAdapter (2)

---

## SRV-13: Trie-based routing — O(k) path lookup

**Task:** #354/#137 | **Commit:** `cc9db23`

### Changes

- **RouteTrie** (NEW) — `TrieNode`-based trie with `insert()`, `lookup()`, `lookupAny()`, `lookupMethods()`
  - `TrieNode`: static `children` map, `paramChild` (`:param`), `wildcardChild` (`*`), `handlers` map
  - Priority via stack ordering: wildcard pushed first, param second, static last (LIFO = static wins)
- **RouteMatch** (NEW) — `handler + pattern + pathParams` (parameters extracted during traversal)
- **BaseWebApplication** — `handlers_` map replaced with `RouteTrie`
  - `registerHandler()` → `trie_.insert()`
  - `findHandler()` → `trie_.lookup()`
  - `pathExists()` → `trie_.lookupAny()`
  - `getAllowedMethods()` → `trie_.lookupMethods()`
  - Removed: `HandlerMatch`, `handlers_` map, `hasParameters()`
- **IRequest** — `setPathParams(const map<string,string>&)`: trie injects params directly
- **SimpleRequest** — `setPathParams()` stores params in `pathParams_` map; `getPathParam()` uses map first
- **BeastRequestAdapter** — same as SimpleRequest
- **PathParamExtractor** — kept for backward compat (used when `pathParams_` is empty)

### Performance

- Before: O(log n) exact + O(n) wildcard scan per request
- After: O(k) single traversal (k = path segments, independent of route count)

### Tests

12 new RouteTrie tests: exact match, named param, wildcard, priority (static > param > wildcard), 404, 405, multiple params, segment mismatch, getAllowedMethods, lookupAny, empty trie

---

## Test Summary

| Stage | Total | New | Status |
|---|---|---|---|
| Before SRV-11 | 448 | — | All pass |
| After SRV-11 | 467 | +19 | All pass |
| After SRV-13 | 479 | +12 | All pass |

---

## Files Changed

### New files
- `microservice-core/include/application/RouteTrie.hpp`
- `microservice-core/src/RouteTrie.cpp`
- `microservice-core/tests/RouteTrieTest.cpp`

### Modified files
- `microservice-core/include/domain/IRequest.hpp`
- `microservice-core/include/adapters/primary/RouteMatcher.hpp`
- `microservice-core/include/adapters/secondary/SimpleRequest.hpp`
- `microservice-core/include/application/BaseWebApplication.hpp`
- `microservice-core/include/util/PathParamExtractor.hpp`
- `microservice-core/src/BaseWebApplication.cpp`
- `microservice-core/src/RouteMatcher.cpp`
- `microservice-core/src/PathParamExtractor.cpp`
- `microservice-core/CMakeLists.txt`
- `microservice-core/tests/CMakeLists.txt`
- `microservice-core/tests/RouteMatcherTest.cpp`
- `microservice-core/tests/SimpleRequestTest.cpp`
- `microservice-core/tests/BaseWebApplicationTest.cpp`
- `microservice-boost/include/adapters/primary/BeastRequestAdapter.hpp`
- `microservice-boost/src/BeastRequestAdapter.cpp`
- `microservice-boost/src/SplunkLogger.cpp`
- `microservice-boost/tests/BeastRequestAdapterTest.cpp`

---

*This file will be merged into CHANGELOG.md at release time.*
