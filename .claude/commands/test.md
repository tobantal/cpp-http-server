Run project tests. Usage: /test [module] [type]

Arguments:
- module: all | core | boost (default: all)
- type: unit | coverage (default: unit)

Examples:
- /test — run all unit tests
- /test core — run microservice-core unit tests
- /test coverage — run tests with coverage report

---

For unit tests, configure and build with CMake then run ctest:

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build
cd build && ctest --verbose
```

For coverage report:

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Debug -DCMAKE_CXX_FLAGS="--coverage -g -O0"
cmake --build build
cd build && ctest --verbose
gcovr --root .. --exclude '.*CMakeFiles/.*' --exclude '.*/tests/.*' --exclude '.*_deps/.*' --html-details coverage.html --print-summary
```