# Nexus

Nexus is a C++20 in-memory key-value storage demo built from custom data structures.

The project combines a concurrent hash map, an LRU cache and a small memory pool behind a simple `KvServer` facade. The executable populates the store, reads the keys back and prints basic throughput and cache statistics.

## Why this project exists

Nexus is meant to show systems-style C++: templates, RAII, move semantics, atomics, mutexes and data structure design. It is not trying to replace a real database. It is a compact project that makes the concurrency and cache behavior visible in code.

## Components

| Component | Purpose |
| --- | --- |
| `ConcurrentMap` | Open-addressed hash map with per-slot synchronization |
| `LruCache` | Hot-key cache with capacity and TTL behavior |
| `MemoryPool` | Simple reusable allocation helper |
| `KvServer` | Small facade combining storage and cache |

## Build

You need CMake and a C++20 compiler.

With Ninja:

```bash
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

With Visual Studio Build Tools:

```bash
cmake -S . -B build -G "Visual Studio 17 2022"
cmake --build build --config Release
```

## Test

Tests use GoogleTest through CMake FetchContent.

```bash
ctest --test-dir build --output-on-failure
```

## Demo

```bash
./build/nexus-server --capacity 1048576 --cache 65536 --ttl 60
```

On Windows with Visual Studio generator, the executable is usually under `build/Release/nexus-server.exe`.

## Code map

- `include/nexus/concurrent_map.hpp`: concurrent hash map.
- `include/nexus/lru_cache.hpp`: LRU cache.
- `include/nexus/memory_pool.hpp`: reusable memory pool.
- `src/server.hpp` and `src/server.cpp`: key-value server facade.
- `src/main.cpp`: benchmark/demo entry point.
- `tests`: unit tests for the main data structures.
