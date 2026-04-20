# CI Extended Configuration (Archived)

> Бывшая расширенная конфигурация CI из SRV-38. Убрана для ускорения CI.
> Вернуть опционально, когда потребуется более глубокая проверка качества.

## Убранные джобы

### macOS Clang Release

```yaml
- os: macos-latest
  compiler: clang
  build-type: Release
  coverage: false
  hardening: false
```

Шаг установки:
```yaml
- name: Install dependencies (macOS)
  if: runner.os == 'macOS'
  run: brew install cmake ninja ccache
```

### GCC Release + Hardening

```yaml
- os: ubuntu-latest
  compiler: gcc
  build-type: Release
  coverage: false
  hardening: true
```

Конфигурация hardening:
```yaml
if [ "${{ matrix.hardening }}" = "true" ]; then
  CXX_FLAGS="-D_FORTIFY_SOURCE=2 -fstack-protector-strong -D_GLIBCXX_ASSERTIONS"
fi
```

### Sanitizers (ASan, UBSan, TSan)

```yaml
sanitize:
  runs-on: ubuntu-latest
  strategy:
    fail-fast: false
    matrix:
      sanitizer: [address, undefined, thread]
  steps:
    - name: Checkout
      uses: actions/checkout@v4
    - name: Install dependencies
      run: sudo apt-get update && sudo apt-get install -y cmake ninja-build clang ccache
    - name: Configure ccache
      run: |
        ccache --max-size=500M
        ccache --set-config=compression=true
        echo "CCACHE_DIR=$HOME/.ccache" >> $GITHUB_ENV
    - name: Cache ccache
      uses: actions/cache@v4
      with:
        path: ~/.ccache
        key: ccache-linux-clang-sanitize-${{ matrix.sanitizer }}-${{ github.run_id }}
        restore-keys: |
          ccache-linux-clang-sanitize-${{ matrix.sanitizer }}-
    - name: Cache FetchContent dependencies
      uses: actions/cache@v4
      with:
        path: build/_deps
        key: deps-linux-clang-sanitize-${{ matrix.sanitizer }}-${{ hashFiles('CMakeLists.txt', '*/CMakeLists.txt') }}
        restore-keys: |
          deps-linux-clang-sanitize-${{ matrix.sanitizer }}-
    - name: Configure with ${{ matrix.sanitizer }} sanitizer
      run: |
        cmake -B build \
          -DCMAKE_BUILD_TYPE=Debug \
          -DCMAKE_CXX_FLAGS="-fsanitize=${{ matrix.sanitizer }} -fno-omit-frame-pointer -g" \
          -DCMAKE_CXX_COMPILER_LAUNCHER=ccache \
          -DCMAKE_POLICY_VERSION_MINIMUM=3.5 \
          -G Ninja
      env:
        CC: clang
        CXX: clang++
    - name: Build
      run: cmake --build build
    - name: Test with ${{ matrix.sanitizer }} sanitizer
      run: ctest --test-dir build --output-on-failure
```

## Зачем нужны

- **macOS**: кросс-платформенная проверка (API的差异, filesystem paths)
- **Hardening**: безопасность бинарника (`_FORTIFY_SOURCE`, stack-protector)
- **ASan**: use-after-free, heap/stack buffer overflow, memory leak
- **UBSan**: undefined behavior (signed overflow, null deref, misalignment)
- **TSan**: data race detection (критично для `std::atomic`, `std::mutex`, `std::thread`)