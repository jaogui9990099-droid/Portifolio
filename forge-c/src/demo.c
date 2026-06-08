#include "forge/forge.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#  define WIN32_LEAN_AND_MEAN
#  include <windows.h>
static double now_ms(void) {
    LARGE_INTEGER freq, count;
    QueryPerformanceFrequency(&freq);
    QueryPerformanceCounter(&count);
    return (double)count.QuadPart * 1000.0 / (double)freq.QuadPart;
}
#else
#  include <time.h>
static double now_ms(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (double)ts.tv_sec * 1e3 + (double)ts.tv_nsec * 1e-6;
}
#endif

#define ITERATIONS 1000000

static void bench_arena(void) {
    forge_arena_t arena;
    if (forge_arena_init(&arena, 16 * 1024 * 1024) != 0) {
        fputs("arena init failed\n", stderr);
        return;
    }

    double t0 = now_ms();
    for (int i = 0; i < ITERATIONS; ++i) {
        void* p = forge_arena_alloc(&arena, 64);
        (void)p;
        if (i % 100000 == 0) forge_arena_reset(&arena);
    }
    double elapsed = now_ms() - t0;

    forge_arena_stats_t s = forge_arena_stats(&arena);
    printf("Arena  -- %d ops in %.2f ms  |  capacity=%zu overflow=%zu\n",
           ITERATIONS, elapsed, s.capacity, s.overflow_count);

    forge_arena_destroy(&arena);
}

static void bench_pool(void) {
    forge_pool_t pool;
    if (forge_pool_init(&pool, 64, 4096) != 0) {
        fputs("pool init failed\n", stderr);
        return;
    }

    size_t  cap  = pool.capacity;
    void**  ptrs = (void**)malloc(sizeof(void*) * cap);

    double t0    = now_ms();
    long rounds  = ITERATIONS / (long)cap;
    for (long r = 0; r < rounds; ++r) {
        for (size_t i = 0; i < cap; ++i) ptrs[i] = forge_pool_alloc(&pool);
        for (size_t i = 0; i < cap; ++i) forge_pool_free(&pool, ptrs[i]);
    }
    double elapsed = now_ms() - t0;

    forge_pool_stats_t s = forge_pool_stats(&pool);
    printf("Pool   -- %ld rounds x %zu in %.2f ms  |  peak=%zu allocs=%zu\n",
           rounds, cap, elapsed, s.peak, s.total_allocs);

    free(ptrs);
    forge_pool_destroy(&pool);
}

static void bench_slab(void) {
    forge_slab_t slab;
    if (forge_slab_init(&slab) != 0) {
        fputs("slab init failed\n", stderr);
        return;
    }

    static const size_t sizes[] = {16, 32, 64, 128, 256};
    const int nsizes = (int)(sizeof(sizes) / sizeof(sizes[0]));

    double t0 = now_ms();
    for (int i = 0; i < ITERATIONS; ++i) {
        size_t sz = sizes[i % nsizes];
        void*  p  = forge_slab_alloc(&slab, sz);
        if (p) forge_slab_free(&slab, p, sz);
    }
    double elapsed = now_ms() - t0;

    forge_slab_stats_t s = forge_slab_stats(&slab);
    printf("Slab   -- %d ops in %.2f ms  |  classes=%d in_use=%zu\n",
           ITERATIONS, elapsed, s.class_count, s.total_in_use);

    forge_slab_destroy(&slab);
}

int main(void) {
    printf("forge v%s -- allocator benchmark (%d iterations)\n\n",
           forge_version_string(), ITERATIONS);
    bench_arena();
    bench_pool();
    bench_slab();
    return 0;
}
