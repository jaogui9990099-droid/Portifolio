# Forge

Forge is a small C11 memory allocation toolkit.

It contains three allocator styles: an arena allocator for short-lived grouped allocations, a fixed-size pool allocator for repeated objects, and a slab allocator for several small allocation classes. There is also a benchmark-style demo and unit tests for the main behavior.

## Why this project exists

This project is about low-level programming: memory layout, alignment, ownership, reset behavior and predictable allocation patterns. It is intentionally written in C so the code has to be explicit about what owns memory and when memory is reused.

## Components

| Component | Purpose |
| --- | --- |
| Arena | Fast bump allocations that can be reset together |
| Pool | Fixed-size object reuse |
| Slab | Multiple size classes for small allocations |

## Build

You need CMake and a C compiler.

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

```bash
ctest --test-dir build --output-on-failure
```

## Demo

```bash
./build/forge-demo
```

On Windows with Visual Studio generator, the executable is usually under `build/Release/forge-demo.exe`.

## Code map

- `include/forge/arena.h` and `src/arena.c`: arena allocator.
- `include/forge/pool.h` and `src/pool.c`: fixed-size pool allocator.
- `include/forge/slab.h` and `src/slab.c`: slab allocator.
- `src/demo.c`: simple allocation benchmark.
- `tests`: unit tests using the small bundled Unity-style test harness.
