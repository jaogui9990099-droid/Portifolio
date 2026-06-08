# Portfolio

Small systems projects focused on backend architecture, memory management and concurrent data structures.

This repository is intentionally split by language. Each project is self-contained and has its own build instructions.

## Projects

| Project | Language | What it shows |
| --- | --- | --- |
| [Meridian](./meridian-java) | Java 21 / Spring Boot | REST API design, job scheduling, priority queues, retries, tests |
| [Forge](./forge-c) | C11 | Arena, pool and slab allocators, manual memory management, CMake, unit tests |
| [Nexus](./nexus-cpp) | C++20 | Concurrent map, LRU cache, memory pool, small key-value server demo |

## Notes

These are portfolio projects, not production libraries. The goal is to show how I structure code, isolate responsibilities, test behavior and build small systems from scratch.

The Java project can be tested with Maven directly. The C and C++ projects require a configured compiler toolchain, such as GCC/Clang with Ninja or Visual Studio Build Tools on Windows.

